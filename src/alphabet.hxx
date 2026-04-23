#ifndef ALPHABET_HXX__
#define ALPHABET_HXX__

#include "typelist.hxx"

// === snippet begin ===
namespace onre {
namespace impl {

/* === compile-time alphabet and helper function === */
constexpr size_t nr_byte = 256;

constexpr std::array<bool, nr_byte> make_valid_table() {
  std::array<bool, nr_byte> table {};
  for (size_t i = 0; i < nr_byte; i++) {
    table[i] = i != '|' && i != '*' && i != '+' && i != '?'
      && i != '(' && i != ')' && i != '[' && i != ']' && i != '\\' && i != '.';
  }
  return table;
}

constexpr auto valid_table = make_valid_table();

constexpr bool is_valid_char(uint8_t ch) {
  return valid_table[ch];
}

constexpr bool is_in_class_char(uint8_t ch) {
  return ch != ']';
}

template <uint8_t c>
struct Char;

template <uint8_t Start, uint8_t End>
struct BuildCharList {
  static_assert(Start <= End, "invalid char range");

  template <typename Indices>
  struct Impl;
  template <size_t... Is>
  struct Impl<std::index_sequence<Is...>> {
    using type = TypeList<Char<Start + Is>...>;
  };

  using type = typename Impl<std::make_index_sequence<End - Start + 1>>::type;
};

template<typename CharList, typename Alphabet>
struct CharListNegation {
  template <typename Char>
  struct NotInList {
    static constexpr bool value = !CharList::template Contains<Char>;
  };
  using type = typename Filter<NotInList, Alphabet>::type;
};

using Alphabet = typename BuildCharList<0x00, 0xFF>::type;

using Utf8FirstByteAlphabet = typename Join<
  typename BuildCharList<0x00, 0x7F>::type,
  typename Join<
    typename BuildCharList<0xC2, 0xDF>::type,
    typename Join<
      typename BuildCharList<0xE0, 0xEF>::type,
      typename BuildCharList<0xF0, 0xF4>::type
    >::type
  >::type
>::type;
using Utf8ContinuationByteAlphabet = typename BuildCharList<0x80, 0xBF>::type;

template<typename CharT>
struct NotWildcardExcluded {
  static constexpr bool value = !std::is_same_v<CharT, Char<'\n'>>
    && !std::is_same_v<CharT, Char<'\r'>>;
};
#ifdef ONRE_DOTALL
  using WildcardAlphabet = Utf8FirstByteAlphabet;
#else
  using WildcardAlphabet = typename Filter<NotWildcardExcluded, Utf8FirstByteAlphabet>::type;
#endif

} /* namespace impl */
} /* namespace onre */

// === snippet end ===

#endif