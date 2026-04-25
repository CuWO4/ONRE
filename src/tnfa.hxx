#ifndef TNFA_HXX__
#define TNFA_HXX__

#include "typelist.hxx"
#include "reg.hxx"
#include "action.hxx"
#include "simplify.hxx"
#include "fixedstring.hxx"

// === snippet begin ===
namespace onre {
namespace impl {

/*
 * for slower O(|s| * #capture * 2^|pattern|) matching with capture group.
 * the matching time is still linear in the length of the input string,
 * but is related to the number of capture group (linear) and the length
 * of the pattern (in the worst case exponential), and have  a bigger O(1).
 * compile such automata would take considerably more time than DFA.
 */
namespace tnfa {

/* === v notation === */
template<typename List, typename Action>
struct ProductAction {
  template <typename A>
  struct AddAction {
    using type = typename CatAction<A, Action>::type;
  };
  using type = typename Map<AddAction, List>::type;
};
template<typename List1, typename List2, typename Acc>
struct Product;
template<
  typename List1,
  typename Head,
  typename... Tails,
  typename Acc
>
struct Product<List1, TypeList<Head, Tails...>, Acc> {
  using Tmp = typename ProductAction<List1, Head>::type;
  using NextAcc = typename JoinUnique<Acc, Tmp>::type;
  using type = typename Product<List1, TypeList<Tails...>, NextAcc>::type;
};
template<typename List1, typename Acc>
struct Product<List1, TypeList<>, Acc> {
  using type = Acc;
};

template<typename RE>
struct v {
  using type = TypeList<>;
};
template<>
struct v<Epsilon> {
  using type = TypeList<Omega>;
};
template<size_t I>
struct v<SetSlot<I>> {
  using type = TypeList<Set<I>>;
};
template<typename R, typename S>
struct v<Or<R, S>> {
  using type = typename JoinUnique<typename v<R>::type, typename v<S>::type>::type;
};
template<typename R, typename S>
struct v<Concat<R, S>> {
  using type = typename Product<typename v<R>::type, typename v<S>::type, TypeList<>>::type;
};
template<typename R>
struct v<Closure<R>> {
  using type = TypeList<Omega>;
};

/* === extended brzozowski derivative === */
template <typename Remain, typename Action>
struct DerivedPair {
  using remain = Remain; using action = Action;
};

template <typename RE, uint8_t C>
struct Derivative;
/* d0/dx de/dx = d<i>/dx = 0 */
template <uint8_t C>
struct Derivative<EmptySet, C> {
  using type = TypeList<>;
};
template <uint8_t C>
struct Derivative<Epsilon, C> {
  using type = TypeList<>;
};
template <size_t I, uint8_t C>
struct Derivative<SetSlot<I>, C> {
  using type = TypeList<>;
};
/* dy/dx = x == y ? {(e, o)} : 0 */
template <uint8_t y, uint8_t C>
struct Derivative<Char<y>, C> {
  using type = typename std::conditional<
    y == C,
    TypeList<DerivedPair<Epsilon, Omega>>,
    TypeList<>
  >::type;
};
template <uint8_t C>
struct Derivative<Wildcard, C> {
  using Dispatch = typename std::conditional<
    0x00 <= C && C <= 0x7F,
    Epsilon,
    typename std::conditional<
      0xC2 <= C && C <= 0xDF,
      Wildcard1,
      typename std::conditional<
        0xE0 <= C && C <= 0xEF,
        Wildcard2,
        typename std::conditional<
          0xF0 <= C && C <= 0xF4,
          Wildcard3,
          EmptySet
        >::type
      >::type
    >::type
  >::type;
  #ifdef ONRE_DOTALL
    using type = TypeList<DerivedPair<Dispatch, Omega>>;
  #else
    using type = typename std::conditional<
      C == '\n' || C == '\r',
      TypeList<>,
      TypeList<DerivedPair<Dispatch, Omega>>
    >::type;
  #endif
};
template <uint8_t C>
struct Derivative<Wildcard3, C> {
  using type = typename std::conditional<
    0x80 <= C && C <= 0xBF,
    TypeList<DerivedPair<Wildcard2, Omega>>,
    TypeList<>
  >::type;
};
template <uint8_t C>
struct Derivative<Wildcard2, C> {
  using type = typename std::conditional<
    0x80 <= C && C <= 0xBF,
    TypeList<DerivedPair<Wildcard1, Omega>>,
    TypeList<>
  >::type;
};
template <uint8_t C>
struct Derivative<Wildcard1, C> {
  using type = typename std::conditional<
    0x80 <= C && C <= 0xBF,
    TypeList<DerivedPair<Epsilon, Omega>>,
    TypeList<>
  >::type;
};
template <typename CharList, uint8_t C>
struct Derivative<Except<CharList>, C> {
  using type = typename std::conditional<
    0x00 <= C && C <= 0x7F,
    typename std::conditional<
      CharList::template Contains<Char<C>>,
      TypeList<>,
      TypeList<DerivedPair<Epsilon, Omega>>
    >::type,
    typename std::conditional<
      0xC2 <= C && C <= 0xDF,
      TypeList<DerivedPair<Wildcard1, Omega>>,
      typename std::conditional<
        0xE0 <= C && C <= 0xEF,
        TypeList<DerivedPair<Wildcard2, Omega>>,
        typename std::conditional<
          0xF0 <= C && C <= 0xF4,
          TypeList<DerivedPair<Wildcard3, Omega>>,
          TypeList<>
        >::type
      >::type
    >::type
  >::type;
};
/* d(R|S)/dx = dR/dx U dS/dx */
template <typename R, typename S, uint8_t C>
struct Derivative<Or<R, S>, C> {
  using dr = typename Derivative<R, C>::type;
  using ds = typename Derivative<S, C>::type;
  using type = typename JoinUnique<dr, ds>::type;
};
/* d(RS)/dx = {(R'S, a):(R',a) in dR/dx} U {(S', ab):a in v(R), (S', b) in dS/dx} */
template <typename R, typename S, uint8_t C>
struct Derivative<Concat<R, S>, C> {
  template <typename Pair>
  struct MapFunc1 {
    using type = DerivedPair<
      typename Simplify<Concat<typename Pair::remain, S>>::type,
      typename Pair::action
    >;
  };
  using Part1 = typename Map<MapFunc1, typename Derivative<R, C>::type>::type;

  template <typename Acc, typename vRList, typename SDList>
  struct Part2Generator;
  template <typename Acc, typename SDList>
  struct Part2Generator<Acc, TypeList<>, SDList> {
    using type = Acc;
  };
  template <typename Acc, typename vRHead, typename... vRTails, typename SDList>
  struct Part2Generator<Acc, TypeList<vRHead, vRTails...>, SDList> {
    template <typename Pair>
    struct MapFunc2 {
      using type = DerivedPair<
        typename Pair::remain,
        typename CatAction<vRHead, typename Pair::action>::type
      >;
    };
    using type = typename Part2Generator<
      typename JoinUnique<Acc, typename Map<MapFunc2, SDList>::type>::type,
      TypeList<vRTails...>,
      SDList
    >::type;
  };
  using Part2 = typename Part2Generator<
    TypeList<>,
    typename v<R>::type,
    typename Derivative<S, C>::type
  >::type;

  using type = typename JoinUnique<Part1, Part2>::type;
};
/* d(R*)/dx = {(R'R*,a):(R',a) in dR/dx} */
template <typename R, uint8_t C>
struct Derivative<Closure<R>, C> {
  template <typename Pair>
  struct MapFunc {
    using type = DerivedPair<
      typename Simplify<Concat<typename Pair::remain, Closure<R>>>::type,
      typename Pair::action
    >;
  };

  using type = typename Map<MapFunc, typename Derivative<R, C>::type>::type;
};

/* === TNFA builder === */
template<typename R>
struct State {
  using re = R;
  static constexpr bool accepting = Nullable<R>::value;
  using AcceptActions = typename v<R>::type;
};

template<size_t From, uint8_t C, typename Action, size_t To>
struct Edge {
  static constexpr std::size_t from = From;
  static constexpr uint8_t ch = C;
  using action = Action;
  static constexpr size_t to = To;
};

template<uint8_t C, typename State, typename Action>
struct CharStateAction {
  static constexpr uint8_t c = C;
  using state = State;
  using action = Action;
};

template<typename CharStateActionAcc, typename State, typename Alphabet>
struct DerivNewStates;
template<typename Acc, typename S>
struct DerivNewStates<Acc, S, TypeList<>> {
  using type = Acc;
};

template<typename Acc, typename S, uint8_t C, typename... Tails>
struct DerivNewStates<Acc, S, TypeList<Char<C>, Tails...>> {
  template<typename RemainActionPair>
  struct AddChar {
    using type = CharStateAction<
      C,
      State<typename RemainActionPair::remain>,
      typename RemainActionPair::action
    >;
  };

  using Der = typename Derivative<typename S::re, C>::type;
  using type = typename std::conditional<
    std::is_same_v<Der, TypeList<>>,
    std::type_identity<Acc>,
    DerivNewStates<
      typename CatList<Acc, typename Map<AddChar, Der>::type>::type,
      S,
      TypeList<Tails...>
    >
  >::type::type;
};

template<
  typename StateAcc,
  typename EdgeAcc,
  typename ToBeProcessList,
  typename StartState,
  typename NewCharStates
>
struct PushNewStates;
template<typename SA, typename EA, typename TBP, typename StartState>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<>> {
  using StateAcc = SA;
  using EdgeAcc = EA;
  using ToBeProcessList = TBP;
};
template<
  typename SA,
  typename EA,
  typename TBP,
  typename StartState,
  typename HeadTuple,
  typename... TailPairs
>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<HeadTuple, TailPairs...>> {
  using FromState = StartState;
  using ToState = typename HeadTuple::state;
  static constexpr uint8_t C = HeadTuple::c;
  using Action = typename HeadTuple::action;
  static constexpr bool IsStateNew = !SA::template Contains<ToState>;
  using NextStateAcc = typename PushBackUnique<SA, ToState>::type;
  using NextToBeProcessList = typename std::conditional<
    IsStateNew,
    PushBack<TBP, ToState>,
    std::type_identity<TBP>
  >::type::type;
  using NextEdgeAcc = typename PushBack<
    EA,
    Edge<
      NextStateAcc::template IndexOf<FromState>,
      C,
      Action,
      NextStateAcc::template IndexOf<ToState>
    >
  >::type;
  using NextIt = PushNewStates<
    NextStateAcc, NextEdgeAcc, NextToBeProcessList, StartState, TypeList<TailPairs...>
  >;
  using StateAcc = typename NextIt::StateAcc;
  using EdgeAcc = typename NextIt::EdgeAcc;
  using ToBeProcessList = typename NextIt::ToBeProcessList;
};

template<typename StateAcc, typename EdgeAcc, typename ToBeProcessList>
struct BuildTNFA;

template<typename StateAcc, typename EdgeAcc>
struct BuildTNFA<StateAcc, EdgeAcc, TypeList<>> {
  using States = StateAcc;
  using Edges = EdgeAcc;
};

template<typename StateAcc, typename EdgeAcc, typename StateHead, typename... StateTails>
struct BuildTNFA<StateAcc, EdgeAcc, TypeList<StateHead, StateTails...>> {
  using NewCharStates = typename DerivNewStates<
    TypeList<>,
    StateHead,
    typename First<typename StateHead::re, TypeList<>>::type
  >::type;
  using Processed = PushNewStates<
    StateAcc, EdgeAcc, TypeList<StateTails...>, StateHead, NewCharStates
  >;
  using NextIt = BuildTNFA<
    typename Processed::StateAcc,
    typename Processed::EdgeAcc,
    typename Processed::ToBeProcessList
  >;
  using States = typename NextIt::States;
  using Edges = typename NextIt::Edges;
};

template<typename RE>
struct AllStatesAndEdgesGenerator {
public:
  using type = BuildTNFA<TypeList<State<RE>>, TypeList<>, TypeList<State<RE>>>;
  using States = typename type::States;
  using Edges  = typename type::Edges;
};

/* aliases */
template<typename RE> using AllStateEdgePair = typename AllStatesAndEdgesGenerator<RE>::type;
template<typename RE> using AllStatesList = typename AllStatesAndEdgesGenerator<RE>::States;
template<typename RE> using AllEdgesList  = typename AllStatesAndEdgesGenerator<RE>::Edges;

/*                         table builder                        *
 * === convert sparse graph representation into jump table, === *
 *           action list and other auxiliary structure          */
template<typename RE>
struct NrUsedSlots {
  static constexpr size_t value = 0;
};
template<size_t I>
struct NrUsedSlots<SetSlot<I>> {
  static constexpr size_t value = I + 1;
};
template<typename R, typename S>
struct NrUsedSlots<Or<R, S>> {
  static constexpr size_t value = std::max(NrUsedSlots<R>::value, NrUsedSlots<S>::value);
};
template<typename R, typename S>
struct NrUsedSlots<Concat<R, S>> {
  static constexpr size_t value = std::max(NrUsedSlots<R>::value, NrUsedSlots<S>::value);
};
template<typename R>
struct NrUsedSlots<Closure<R>> {
  static constexpr size_t value = NrUsedSlots<R>::value;
};

/* use a compile-time upper bound for action table width */
template<size_t NrStates, typename Edges>
struct MaxTransActionLength;
template<size_t NrStates>
struct MaxTransActionLength<NrStates, TypeList<>> {
  static constexpr size_t value = 0;
};
template<size_t NrStates, typename... Edges>
struct MaxTransActionLength<NrStates, TypeList<Edges...>> {
  static constexpr size_t value = []() {
    std::array<std::array<std::array<size_t, NrStates>, nr_byte>, NrStates> min_len;
    std::array<std::array<std::array<bool, NrStates>, nr_byte>, NrStates> inited;
    for (auto& from_table : inited)
      for (auto& char_table : from_table)
        char_table.fill(false);
    ([&]<typename Edge>(Edge) {
      if (
        inited[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to]
        && min_len[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to] < Edge::action::length
      ) return;
      inited[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to] = true;
      min_len[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to] = Edge::action::length;
    }(Edges{}), ...);
    size_t result = 0;
    for (size_t i = 0; i < NrStates; i++)
      for (size_t ch = 0; ch < nr_byte; ch++)
        for (size_t j = 0; j < NrStates; j++)
          if (inited[i][ch][j] && min_len[i][ch][j] > result)
            result = min_len[i][ch][j];
    return result;
  }();
};

template<typename List>
struct MaxActionLengthInList;
template<>
struct MaxActionLengthInList<TypeList<>> {
  static constexpr size_t value = 0;
};
template<typename... Actions>
struct MaxActionLengthInList<TypeList<Actions...>> {
  static constexpr size_t value = std::max({ Actions::length... });
};

template<typename States>
struct MaxAcceptActionLength;
template<>
struct MaxAcceptActionLength<TypeList<>> {
  static constexpr size_t value = 0;
};
template<typename... States>
struct MaxAcceptActionLength<TypeList<States...>> {
  static constexpr size_t value = std::max({
    MaxActionLengthInList<typename States::AcceptActions>::value...
  });
};

template<size_t NrStates, typename EdgeList>
struct BuildTransTable;
template<size_t NrStates, typename... Edges>
struct BuildTransTable<NrStates, TypeList<Edges...>> {
  static constexpr std::array<
    std::array<std::array<int32_t, NrStates>, nr_byte>,
    NrStates
  > make() {
    std::array<std::array<std::array<int32_t, NrStates>, nr_byte>, NrStates> result{};
    for (auto& state_table : result) for (auto& char_table : state_table) char_table.fill(-1);

    std::array<std::array<size_t, nr_byte>, NrStates> idxes {};
    for (auto& line : idxes) line.fill(0);

    std::array<std::array<std::array<bool, NrStates>, nr_byte>, NrStates> inited{};
    for (auto& from_table : inited)
      for (auto& ch_table : from_table)
        ch_table.fill(false);

    (([&]<typename Edge>(Edge) {
      constexpr size_t from = Edge::from;
      constexpr size_t to   = Edge::to;
      constexpr size_t ch   = static_cast<size_t>(Edge::ch);
      if (inited[from][ch][to]) return;
      inited[from][ch][to] = true;

      auto& idx = idxes[from][ch];
      if (idx >= NrStates) return;
      result[from][ch][idx++] = static_cast<int32_t>(to);
    }(Edges{})), ...);

    return result;
  }
};

template<typename StateList>
struct BuildAcceptTable;
template<typename... States>
struct BuildAcceptTable<TypeList<States...>> {
  static constexpr std::array<bool, sizeof...(States)> make() {
    return std::array<bool, sizeof...(States)>{ States::accepting... };
  }
};

template<size_t NrStates, size_t MaxTransActionLength, typename EdgeList>
struct BuildTransActionTable;
template<size_t NrStates, size_t MaxTransActionLength, typename... Edges>
struct BuildTransActionTable<NrStates, MaxTransActionLength, TypeList<Edges...>> {
  static constexpr std::array<
    std::array<std::array<std::array<int32_t, MaxTransActionLength>, NrStates>, nr_byte>,
    NrStates
  > make() {
    std::array<std::array<std::array<
      std::array<int32_t, MaxTransActionLength>,
      NrStates>, nr_byte>, NrStates
    > result{};
    std::array<std::array<std::array<bool, NrStates>, nr_byte>, NrStates> action_inited{};
    for (auto& from_state_table : result)
      for (auto& char_table : from_state_table)
        for (auto& action_list : char_table)
          action_list.fill(-1);
    for (auto& from_state_table : action_inited)
      for (auto& char_table : from_state_table)
        char_table.fill(false);

    /* use the shortest action among edges sharing same from, ch and to */
    auto existing_len_of = []<size_t L>(const std::array<int32_t, L>& list) constexpr {
      size_t i = 0;
      while (i < L && list[i] != -1) ++i;
      return i;
    };

    (([&]<typename Edge>(Edge) {
      using Action = typename Edge::action;
      /*
       * MaxTransActionLength is generated using the shortest action among
       * edges sharing same from, ch and to. Therefore, if current action
       * is longer than it, it's guaranteed that there exists a shorter action
       * for same from, ch and to, and current action can be ignored safely.
       * On the other hand, if not do so, will trigger array out-of-bound
       * access later.
       */
      if (Action::length > MaxTransActionLength) return;
      auto& action_list = result[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to];
      if (
        action_inited[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to]
        && Action::length > existing_len_of(action_list)
      ) return;
      action_inited[Edge::from][static_cast<size_t>(Edge::ch)][Edge::to] = true;
      action_list.fill(-1);
      if constexpr (is_omega<Action>::value) {
        /* ignore */
      } else if constexpr (is_set<Action>::value) {
        action_list[0] = static_cast<int32_t>(Action::i);
      } else if constexpr (is_seq<Action>::value) {
        [&]<typename... As>(Seq<As...>) {
          size_t idx = 0;
          ((action_list[idx++] = static_cast<int32_t>(As::i)), ...);
        }(Action{});
      } else {
        static_assert(!std::is_same<Action, Action>::value, "unknown action type");
      }
    }(Edges{})), ...);

    return result;
  }
};

template<size_t MaxAcceptActionLength, typename StateList>
struct BuildAcceptActionTable;
template<size_t MaxAcceptActionLength, typename... States>
struct BuildAcceptActionTable<MaxAcceptActionLength, TypeList<States...>> {
  static constexpr std::array<
    std::array<int32_t, MaxAcceptActionLength>,
    sizeof...(States)
  > make() {
    std::array<std::array<int32_t, MaxAcceptActionLength>, sizeof...(States)> result{};
    for (auto& action_list : result) action_list.fill(-1);
    (([&]<typename State>(State){
      auto& action_list = result[TypeList<States...>::template IndexOf<State>];
      // simplify choose the longest if multiple accept actions are possible heuristically
      using Action = typename LongestAction<typename v<typename States::re>::type, Omega, 0>::type;
      if constexpr (is_omega<Action>::value) {
        /* ignore */
      } else if constexpr (is_set<Action>::value) {
        static_assert(MaxAcceptActionLength > 0, "bad max accept action length");
        action_list[0] = static_cast<int32_t>(Action::i);
      } else if constexpr (is_seq<Action>::value) {
        [&]<typename... As>(Seq<As...>) {
          static_assert(MaxAcceptActionLength >= sizeof...(As), "bad max accept action length");
          size_t idx = 0;
          ((action_list[idx++] = static_cast<int32_t>(As::i)), ...);
        }(Action{});
      } else {
        static_assert(!std::is_same<States, States>::value, "unknown action type");
      }
    }(States{})), ...);
    return result;
  }

  template<typename ActionList, typename CurLongestAction, size_t CurShortestLen>
  struct LongestAction;
  template<typename CurLongestAction, size_t CurShortestLen>
  struct LongestAction<TypeList<>, CurLongestAction, CurShortestLen> {
    using type = CurLongestAction;
  };
  template<
    typename HeadAction,
    typename... TailActions,
    typename CurLongestAction,
    size_t CurShortestLen
  >
  struct LongestAction<TypeList<HeadAction, TailActions...>, CurLongestAction, CurShortestLen> {
    using type = typename std::conditional<
      (HeadAction::length > CurShortestLen),
      LongestAction<TypeList<TailActions...>, HeadAction, HeadAction::length>,
      LongestAction<TypeList<TailActions...>, CurLongestAction, CurShortestLen>
    >::type::type;
  };
};

template<size_t Value>
struct Num {
  static constexpr size_t value = Value;
};
template<size_t X, size_t Y>
struct NumPair {
  static constexpr size_t x = X, y = Y;
};
template<
  typename Acc,
  FixedString Pattern,
  size_t Idx,
  size_t NrSeenGroup,
  typename OpeningGroupsIdx,
  typename ClosedGroups
>
struct MutualGroups {
  struct Impl {
    struct Open {
      static constexpr size_t OpenedIdx = NrSeenGroup;
      template<typename ClosedIdx>
      struct MapFunc {
        using type = NumPair<ClosedIdx::value, OpenedIdx>;
      };
      using NextAcc = typename JoinUnique<Acc, typename Map<MapFunc, ClosedGroups>::type>::type;
      using NextOpeningGroupsIdx = typename PushFront<OpeningGroupsIdx, Num<OpenedIdx>>::type;
      using type = typename MutualGroups<
        NextAcc, Pattern, Idx + 1, NrSeenGroup + 1, NextOpeningGroupsIdx, ClosedGroups
      >::type;
    };
    struct Close {
      static_assert(OpeningGroupsIdx::length > 0, "MutualGroups: unmatched ')'");
      static constexpr size_t ClosedIdx = OpeningGroupsIdx::template At<0>::value;
      using NextOpeningGroupsIdx = typename PopFront<OpeningGroupsIdx>::type;
      using NextClosedGroups = typename PushBack<ClosedGroups, Num<ClosedIdx>>::type;
      using type = typename MutualGroups<
        Acc, Pattern, Idx + 1, NrSeenGroup, NextOpeningGroupsIdx, NextClosedGroups
      >::type;
    };
    struct Other {
      using type = typename MutualGroups<
        Acc, Pattern, Idx + 1, NrSeenGroup, OpeningGroupsIdx, ClosedGroups
      >::type;
    };
    using type = typename std::conditional<
      Pattern[Idx] == '(',
      Open,
      typename std::conditional<Pattern[Idx] == ')', Close, Other>::type
    >::type::type;
  };
  using type = typename std::conditional<
    Idx >= Pattern.length,
    std::type_identity<Acc>,
    Impl
  >::type::type;
};

} /* namespace tnfa */
} /* namespace impl */
} /* namespace onre */

// === snippet end ===



#endif