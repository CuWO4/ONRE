#ifndef INTERFACE_HXX__
#define INTERFACE_HXX__

#include "regexscan.hxx"
#include "dfa.hxx"
#include "tnfa.hxx"

// === snippet begin ===
namespace onre {

/* === interface === */
template<impl::FixedString Pattern>
inline bool match(std::string_view str);
template<impl::FixedString Pattern>
inline std::string replace(std::string_view rule, std::string_view str);

template<impl::FixedString Pattern>
inline bool match(std::string_view str) {
  using Re = typename impl::RegexScan<Pattern>::type;
  using NoActionRe = typename impl::dfa::RemoveAllAction<Re>::type;
  using DFAStatesList = impl::dfa::AllStatesList<NoActionRe>;
  using DFAEdgesList  = impl::dfa::AllEdgesList<NoActionRe>;
  static constexpr std::size_t nr_dfa_states = DFAStatesList::length;
  static constexpr auto dfa_trans_table
    = impl::dfa::BuildTable<nr_dfa_states, DFAEdgesList>::make();
  static constexpr auto dfa_is_accept_states = impl::dfa::BuildAccepts<DFAStatesList>::make();

  std::size_t state = 0;
  for (uint8_t uch : str) {
    int32_t nxt = dfa_trans_table[state][static_cast<std::size_t>(uch)];
    if (nxt < 0) return false;
    state = static_cast<std::size_t>(nxt);
  }
  return dfa_is_accept_states[state];
}

template<impl::FixedString Pattern>
inline std::string replace(std::string_view replace_rule, std::string_view str) {
  using Re = typename impl::RegexScan<Pattern>::type;
  using StateList = impl::tnfa::AllStatesList<Re>;
  using EdgeList  = impl::tnfa::AllEdgesList<Re>;
  static constexpr std::size_t nr_states = StateList::length;
  static constexpr std::size_t nr_used_slots = impl::tnfa::NrUsedSlots<Re>::value;
  static constexpr std::size_t max_trans_action_length
    = impl::tnfa::MaxTransActionLength<nr_states, EdgeList>::value;
  static constexpr std::size_t max_accept_action_length
    = impl::tnfa::MaxAcceptActionLength<StateList>::value;
  static constexpr std::size_t nr_capture_group = nr_used_slots / 2;

  /* S x Alphabet -> int32_t[], -1 represent no trans */
  static constexpr auto trans_table = impl::tnfa::BuildTransTable<nr_states, EdgeList>::make();
  /* S -> bool */
  static constexpr auto accept_table = impl::tnfa::BuildAcceptTable<StateList>::make();
  /* S x Alphabet x S -> int32_t[], -1 represent no action */
  static constexpr auto trans_action_table
    = impl::tnfa::BuildTransActionTable<nr_states, max_trans_action_length, EdgeList>::make();
  /* S -> int32_t[], -1 represent no action */
  static constexpr auto accept_action_table
    = impl::tnfa::BuildAcceptActionTable<max_accept_action_length, StateList>::make();

  using SlotLine = std::array<int32_t, nr_used_slots>;
  using SlotFile = std::array<SlotLine, nr_states>;

  // use macro to force inline, while some compilers (especially older versions)
  // don't perform well with lambda inlining optimizations.
  #define open_time__onre__(line, group_idx) \
    ((line)[(group_idx) << 1])
  #define close_time__onre__(line, group_idx) \
    ((line)[(group_idx) << 1 | 1])
  #define is_opened__onre__(line, group_idx) \
    (open_time__onre__((line), (group_idx)) >= 0)
  #define is_closed__onre__(line, group_idx) \
    (close_time__onre__((line), (group_idx)) >= 0)
  #define group_len__onre__(line, group_idx) \
    (close_time__onre__((line), (group_idx)) - open_time__onre__((line), (group_idx)))
  #define is_digit__onre__(ch) ('0' <= (ch) && (ch) <= '9')

  // heuristically choose a slot configuration to try to get longest match
  static auto need_change = [](const SlotLine& old_line, const SlotLine& new_line) {
    for (size_t k = 0; k < nr_capture_group; k++) {
      if (!is_opened__onre__(old_line, k) && !is_opened__onre__(new_line, k)) continue;
      if (is_opened__onre__(old_line, k) && !is_opened__onre__(new_line, k)) return false;
      if (!is_opened__onre__(old_line, k) && is_opened__onre__(new_line, k)) return true;
      if (!is_closed__onre__(old_line, k) && !is_closed__onre__(new_line, k)) {
        if (open_time__onre__(old_line, k) < open_time__onre__(new_line, k)) return false;
        if (open_time__onre__(old_line, k) > open_time__onre__(new_line, k)) return true;
        continue;
      }
      if (!is_closed__onre__(old_line, k) && is_closed__onre__(new_line, k)) return false;
      if (is_closed__onre__(old_line, k) && !is_closed__onre__(new_line, k)) return true;
      if (group_len__onre__(old_line, k) > group_len__onre__(new_line, k)) return false;
      if (group_len__onre__(old_line, k) < group_len__onre__(new_line, k)) return true;
      if (open_time__onre__(old_line, k) > open_time__onre__(new_line, k)) return false;
      if (open_time__onre__(old_line, k) < open_time__onre__(new_line, k)) return true;
    }
    return false;
  };

  // use macro to explicitly express NRVO, significantly improve performance on
  // some compilers.
  #define apply_action__onre__(old_line, actions, p, new_line) \
    do { \
      (new_line) = (old_line); \
      for (const auto& action : (actions)) { \
        if (action < 0) break; \
        (new_line)[action] = (p); \
      } \
    } while (0)


  thread_local static SlotFile slot_file1, slot_file2;
  thread_local static std::array<bool, nr_states> is_state_active1, is_state_active2;
  thread_local static std::vector<size_t> active_states1, active_states2;
  thread_local static SlotLine slot_line_buf;

  SlotFile* cur_slot_file = &slot_file1;
  SlotFile* nxt_slot_file = &slot_file2;
  // use pointer to explicitly eliminate TLS
  for (auto& line : *cur_slot_file) line.fill(-1);
  for (auto& line : *nxt_slot_file) line.fill(-1);
  auto* cur_active_states = &active_states1;
  auto* nxt_active_states = &active_states2;
  cur_active_states->clear();
  cur_active_states->reserve(nr_states);
  nxt_active_states->clear();
  nxt_active_states->reserve(nr_states);
  auto* cur_is_state_active = &is_state_active1;
  auto* nxt_is_state_active = &is_state_active2;
  cur_is_state_active->fill(false);
  nxt_is_state_active->fill(false);

  cur_active_states->push_back(0);
  (*cur_is_state_active)[0] = true;

  for (size_t idx = 0; idx < str.size(); idx++) {
    uint8_t uch = str[idx];

    nxt_active_states->clear();
    nxt_is_state_active->fill(false);

    for (size_t state : *cur_active_states) {
      for (const auto& nxt_state : trans_table[state][static_cast<size_t>(uch)]) {
        if (nxt_state < 0) break;

        if (!(*nxt_is_state_active)[nxt_state]) {
          apply_action__onre__(
            (*cur_slot_file)[state],
            trans_action_table[state][static_cast<size_t>(uch)][nxt_state],
            static_cast<int32_t>(idx),
            (*nxt_slot_file)[nxt_state]
          );
          nxt_active_states->push_back(nxt_state);
          (*nxt_is_state_active)[nxt_state] = true;
          continue;
        }

        apply_action__onre__(
          (*cur_slot_file)[state],
          trans_action_table[state][static_cast<size_t>(uch)][nxt_state],
          static_cast<int32_t>(idx),
          slot_line_buf
        );
        if (need_change((*nxt_slot_file)[nxt_state], slot_line_buf)) {
          (*nxt_slot_file)[nxt_state] = slot_line_buf;
        }
      }
    }

    std::swap(cur_slot_file, nxt_slot_file);
    std::swap(cur_active_states, nxt_active_states);
    std::swap(cur_is_state_active, nxt_is_state_active);
  }

  bool is_final_line_inited = false;
  SlotLine final_line {};
  for (size_t state : *cur_active_states) {
    if (!accept_table[state]) continue;
    if (!is_final_line_inited) {
      apply_action__onre__((*cur_slot_file)[state], accept_action_table[state], str.size(), final_line);
      is_final_line_inited = true;
      continue;
    }
    apply_action__onre__((*cur_slot_file)[state], accept_action_table[state], str.size(), slot_line_buf);
    if (need_change(final_line, slot_line_buf)) final_line = slot_line_buf;
  }

  if (!is_final_line_inited) return "";

  std::string result;
  result.reserve(str.size() + replace_rule.size());

  static auto find_dollar = [](std::string_view str, size_t start_pos) {
    return std::find(str.begin() + start_pos, str.end(), '$') - str.begin();
  };

  for (size_t idx = 0; idx < replace_rule.size(); idx++) {
    size_t nxt_dollar_idx = find_dollar(replace_rule, idx);
    result.append(replace_rule.data() + idx, nxt_dollar_idx - idx);
    idx = nxt_dollar_idx;
    if (idx >= replace_rule.size()) break;
    if (++idx >= replace_rule.size()) return "";
    if (replace_rule[idx] == '$') result.append("$");
    else if (is_digit__onre__(replace_rule[idx])) {
      size_t group_idx = replace_rule[idx] - '0';
      while (idx + 1 < replace_rule.size() && is_digit__onre__(replace_rule[idx + 1])) {
        idx++;
        group_idx = 10 * group_idx + replace_rule[idx] - '0';
      }
      if (group_idx >= nr_capture_group) return "";
      int32_t l = open_time__onre__(final_line, group_idx),
              r = close_time__onre__(final_line, group_idx);
      if (l < 0 || r < 0) continue;

      if (r < l) continue;
      result.append(str.data() + l, static_cast<size_t>(r - l));
    }
    else return "";
  }

  return result;

  #undef open_time__onre__
  #undef close_time__onre__
  #undef is_opened__onre__
  #undef is_closed__onre__
  #undef group_len__onre__
  #undef is_digit__onre__
  #undef apply_action__onre__
}

} /* namespace onre */

// === snippet end ===



#endif