#ifndef REGEXSCAN_HXX__
#define REGEXSCAN_HXX__

#include "typelist.hxx"
#include "fixedstring.hxx"
#include "reg.hxx"
#include "simplify.hxx"
#include "alphabet.hxx"

// === snippet begin ===
namespace onre {
namespace impl {

/* === regex parser === */
/*
  Grammar:
    Regex       := Term ('|' Regex)?
    Term        := Factor Term | (empty)
    Factor      := Atom ('*')? | Atom ('+')? | Atom ('?')?
                | Atom '{' Number ',' '}' | Atom '{' Number ',' Number '}'
                | Atom '{' ',' Number '}'
    Atom        := '(' ('?:')? Regex ')' | CharGroup | CHAR | '.' | '(#?' comment ')'
    CharGroup   := '[' CharSet ']' | '[' '^' CharSet ']'
    CharSet     := CharSetAtom CharSet | CharSetAtom
    CharSetAtom := [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | Escape
    CHAR        := [VALID CHAR] | Escape
    Escape      := '\' [VISIBLE CHAR] | '\' 'x' HexNumber | '\u''{' HexNumber '}'
    Empty input -> Epsilon
*/

/* forward declarations */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseRegex;
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseTerm;
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseFactor;
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseAtom;
template<FixedString Pattern, size_t Pos>
struct ParseCharGroup;
template<FixedString Pattern, size_t Pos>
struct ParseCharSet;
template<FixedString Pattern, size_t Pos>
struct ParseCharSetAtom;
template<FixedString Pattern, size_t Pos>
struct ParseCHAR;
template<FixedString Pattern, size_t Pos>
struct ParseEscape;

constexpr size_t utf8_sequence_length(uint8_t lead) {
  if (lead < 0x80) return 1;
  if ((lead & 0xE0) == 0xC0) return 2;
  if ((lead & 0xF0) == 0xE0) return 3;
  if ((lead & 0xF8) == 0xF0) return 4;
  return 0;
}

template<uint32_t code>
struct CodePoint {
  static constexpr uint32_t value = code;
};

template<FixedString Pattern, size_t Pos>
struct ParseUtf8CodePoint {
  static_assert(Pos < Pattern.length, "unexpected end of pattern");
  static constexpr size_t len = utf8_sequence_length(Pattern[Pos]);
  static_assert(len > 0, "UTF-8 decode error");
  static_assert(Pos + len <= Pattern.length, "unexpected end of pattern");
  static_assert(
    len != 2 || ((Pattern[Pos + 1] & 0xC0) == 0x80),
    "UTF-8 decode error, invalid continuation byte"
  );
  static_assert(
    len != 3 || (((Pattern[Pos + 1] & 0xC0) == 0x80) && (((Pattern[Pos + 2] & 0xC0) == 0x80))),
    "UTF-8 decode error, invalid continuation byte"
  );
  static_assert(
    len != 4 || (
      ((Pattern[Pos + 1] & 0xC0) == 0x80)
      && ((Pattern[Pos + 2] & 0xC0) == 0x80)
      && ((Pattern[Pos + 3] & 0xC0) == 0x80)
    ), "UTF-8 decode error, invalid continuation byte"
  );
  static constexpr uint32_t value = len == 1
    ? Pattern[Pos]
    : len == 2
      ? (((Pattern[Pos] & 0x1F) << 6) | (Pattern[Pos + 1] & 0x3F))
      : len == 3
        ? (((Pattern[Pos] & 0x0F) << 12) | ((Pattern[Pos + 1] & 0x3F) << 6) | (Pattern[Pos + 2] & 0x3F))
        : (((Pattern[Pos] & 0x07) << 18) | ((Pattern[Pos + 1] & 0x3F) << 12)
          | ((Pattern[Pos + 2] & 0x3F) << 6) | (Pattern[Pos + 3] & 0x3F));
  static constexpr size_t next = Pos + len;
  static_assert(value <= 0x10FFFF, "UTF-8 decode error, code point overflow");
};

template<uint32_t Code>
struct BuildUtf8ByteStreamRegex {
  using type = decltype([]{
    if constexpr (Code <= 0x7F) {
      return Char<Code>{};
    } else if constexpr (Code <= 0x7FF) {
      return Concat<Char<0xC0 | (Code >> 6)>, Char<0x80 | (Code & 0x3F)>>{};
    } else if constexpr (Code <= 0xFFFF) {
      return Concat<Concat<
        Char<0xE0 | (Code >> 12)>,
        Char<0x80 | ((Code >> 6) & 0x3F)>>,
        Char<0x80 | (Code & 0x3F)>>{};
    } else /* Code <= 0x10FFFF */ {
      return Concat<Concat<Concat<
        Char<0xF0 | (Code >> 18)>,
        Char<0x80 | ((Code >> 12) & 0x3F)>>,
        Char<0x80 | ((Code >> 6) & 0x3F)>>,
        Char<0x80 | (Code & 0x3F)>>{};
    }
  }());
};

template<uint32_t Start, uint32_t End>
struct BuildCodePointRange {
  static_assert(Start <= End, "invalid code point range");

  template <typename Indices>
  struct Impl;
  template <size_t... Is>
  struct Impl<std::index_sequence<Is...>> {
    using type = TypeList<CodePoint<Start + static_cast<uint32_t>(Is)>...>;
  };

  using type = typename Impl<std::make_index_sequence<End - Start + 1>>::type;
};

template<typename CharList>
struct BuildOrTree {
  template<typename C1, typename C2>
  struct Merge {
    using type = Or<C1, C2>;
  };
  using type = typename RightFold<Merge, CharList, EmptySet>::type;
};

template<FixedString Pattern, size_t Pos, int64_t Acc = 0>
struct ParseDecimal {
  struct is_digit_impl {
    using next_parse = ParseDecimal<Pattern, Pos + 1, 10 * Acc + (Pattern[Pos] - '0')>;
    static constexpr int64_t value = next_parse::value;
    static constexpr size_t next = next_parse::next;
  };
  struct not_digit_impl {
    static constexpr int64_t value = Acc;
    static constexpr size_t next = Pos;
  };
  using chosen = std::conditional_t<
    Pos < Pattern.length && Pattern[Pos] >= '0' && Pattern[Pos] <= '9',
    is_digit_impl,
    not_digit_impl
  >;
  static constexpr int64_t value = chosen::value;
  static constexpr size_t next = chosen::next;
};

template<FixedString Pattern, size_t Pos, size_t N, int64_t Acc = 0>
struct ParseHexN {
  struct is_digit_impl {
    static constexpr uint8_t ch = Pattern[Pos];
    static constexpr int64_t digit_value = (ch >= '0' && ch <= '9')
      ? ch - '0'
      : (ch >= 'A' && ch <= 'F')
        ? ch - 'A' + 10
        : ch - 'a' + 10
    ;
    using next_parse = ParseHexN<Pattern, Pos + 1, N - 1, 16 * Acc + digit_value>;
    static constexpr int64_t value = next_parse::value;
    static constexpr size_t next = next_parse::next;
  };
  struct not_digit_impl {
    static constexpr int64_t value = Acc;
    static constexpr size_t next = Pos;
  };
  using chosen = std::conditional_t<
    (N > 0) && Pos < Pattern.length && (
      (Pattern[Pos] >= '0' && Pattern[Pos] <= '9')
      || (Pattern[Pos] >= 'a' && Pattern[Pos] <= 'f')
      || (Pattern[Pos] >= 'A' && Pattern[Pos] <= 'F')
    ),
    is_digit_impl,
    not_digit_impl
  >;
  static constexpr int64_t value = chosen::value;
  static constexpr size_t next = chosen::next;
};

template<uint8_t C, FixedString Pattern, size_t Pos>
struct EscapeImpl {
  static constexpr uint32_t code = Pattern[Pos + 1];
  using type = Char<Pattern[Pos + 1]>;
  static constexpr size_t next = Pos + 2;
};
using WordList = typename PushBack<typename Join<
  typename BuildCharList<'a', 'z'>::type,
  typename Join<
    typename BuildCharList<'A', 'Z'>::type,
    typename BuildCharList<'0', '9'>::type
  >::type
>::type, Char<'_'>>::type;
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'w', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<WordList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'W', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<typename CharListNegation<WordList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
using DigitalList = typename BuildCharList<'0', '9'>::type;
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'d', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<DigitalList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'D', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<typename CharListNegation<DigitalList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
using WhitespaceList = TypeList<Char<'\t'>, Char<'\n'>, Char<'\v'>, Char<'\f'>, Char<'\r'>, Char<' '>>;
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'s', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<WhitespaceList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'S', Pattern, Pos> {
  static constexpr uint32_t code = -1;
  using type = typename BuildOrTree<typename CharListNegation<WhitespaceList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'n', Pattern, Pos> {
  static constexpr uint32_t code = '\n';
  using type = Char<'\n'>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'t', Pattern, Pos> {
  static constexpr uint32_t code = '\t';
  using type = Char<'\t'>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'f', Pattern, Pos> {
  static constexpr uint32_t code = '\f';
  using type = Char<'\f'>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'r', Pattern, Pos> {
  static constexpr uint32_t code = '\r';
  using type = Char<'\r'>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'x', Pattern, Pos> {
  static_assert(
    Pos + 2 < Pattern.length && (
      (Pattern[Pos + 2] >= '0' && Pattern[Pos + 2] <= '9')
      || (Pattern[Pos + 2] >= 'a' && Pattern[Pos + 2] <= 'f')
      || (Pattern[Pos + 2] >= 'A' && Pattern[Pos + 2] <= 'F')
    ),
    "no value specified for `\\x`"
  );
  using HexParse = ParseHexN<Pattern, Pos + 2, 2>;
  static constexpr uint32_t code = HexParse::value;
  using type = typename BuildUtf8ByteStreamRegex<HexParse::value>::type;
  static constexpr size_t next = HexParse::next;
};

template<FixedString Pattern, size_t Pos, uint32_t Acc, size_t Digits>
struct ParseHexBraced {
  static_assert(Pos < Pattern.length, "ParseEscape: incomplete Unicode escape");
  static constexpr bool is_hex_digit =
    (Pattern[Pos] >= '0' && Pattern[Pos] <= '9')
    || (Pattern[Pos] >= 'A' && Pattern[Pos] <= 'F')
    || (Pattern[Pos] >= 'a' && Pattern[Pos] <= 'f');
  static_assert(Pattern[Pos] == '}' || is_hex_digit, "ParseEscape: invalid Unicode escape");

  struct impl_end {
    static_assert(Digits > 0, "ParseEscape: no value specified for `\\u{`");
    static constexpr uint32_t value = Acc;
    static constexpr size_t next = Pos + 1;
  };

  struct impl_digit {
    static_assert(Digits < 6, "ParseEscape: Unicode code point overflow");
    static constexpr uint8_t ch = Pattern[Pos];
    static constexpr uint32_t digit_value = (ch >= '0' && ch <= '9')
      ? ch - '0'
      : (ch >= 'A' && ch <= 'F')
        ? ch - 'A' + 10
        : ch - 'a' + 10;
    using next_parse = ParseHexBraced<Pattern, Pos + 1, 16 * Acc + digit_value, Digits + 1>;
    static constexpr uint32_t value = next_parse::value;
    static constexpr size_t next = next_parse::next;
  };

  using chosen = std::conditional_t<
    Pattern[Pos] == '}',
    impl_end,
    impl_digit
  >;
  static constexpr uint32_t value = chosen::value;
  static constexpr size_t next = chosen::next;
  static_assert(value <= 0x10FFFF, "ParseEscape: Unicode code point overflow");
};

template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'u', Pattern, Pos> {
  static_assert(Pos + 3 < Pattern.length, "ParseEscape: incomplete Unicode escape");
  static_assert(Pattern[Pos + 2] == '{', "ParseEscape: malformed Unicode escape");
  using HexParse = ParseHexBraced<Pattern, Pos + 3, 0, 0>;
  static constexpr uint32_t code = HexParse::value;
  using type = typename BuildUtf8ByteStreamRegex<HexParse::value>::type;
  static constexpr size_t next = HexParse::next;
};

template <FixedString Pattern, size_t Pos>
struct ParseEscape {
  static_assert(Pos + 1 < Pattern.length, "ParseEscape: cannot find escape character");
  using chosen = EscapeImpl<Pattern[Pos + 1], Pattern, Pos>;
  static constexpr uint32_t code = chosen::code;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCHAR : [VALID CHAR] | Escape */
template <FixedString Pattern, size_t Pos>
struct ParseCHAR {
  static_assert(
    Pos < Pattern.length && (Pattern[Pos] == '\\' || is_valid_char(Pattern[Pos])),
    "ParseCHAR: unknown character"
  );

  struct impl_escape {
    using EscapeParse = ParseEscape<Pattern, Pos>;
    using type = typename EscapeParse::type;
    static constexpr size_t next = EscapeParse::next;
  };

  struct impl_simple {
    using CodePoint = ParseUtf8CodePoint<Pattern, Pos>;
    using type = typename BuildUtf8ByteStreamRegex<CodePoint::value>::type;
    static constexpr size_t next = CodePoint::next;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  using chosen = std::conditional_t<is_escape, impl_escape, impl_simple>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

template<typename List1, typename List2, typename List3, typename List4>
struct ByteListPack {
  using list1 = List1;
  using list2 = List2;
  using list3 = List3;
  using list4 = List4;
};

template<uint32_t Code>
struct Utf8Encoding {
  static constexpr size_t length =
    (Code <= 0x7F) ? 1 :
    (Code <= 0x7FF) ? 2 :
    (Code <= 0xFFFF) ? 3 : 4;
  static constexpr uint8_t b0 =
    (length == 1) ? static_cast<uint8_t>(Code)
    : (length == 2) ? static_cast<uint8_t>(0xC0 | (Code >> 6))
    : (length == 3) ? static_cast<uint8_t>(0xE0 | (Code >> 12))
    : static_cast<uint8_t>(0xF0 | (Code >> 18));
  static constexpr uint8_t b1 =
    (length == 1) ? 0
    : (length == 2) ? static_cast<uint8_t>(0x80 | (Code & 0x3F))
    : static_cast<uint8_t>(0x80 | ((Code >> 6) & 0x3F));
  static constexpr uint8_t b2 =
    (length <= 2) ? 0
    : (length == 3) ? static_cast<uint8_t>(0x80 | (Code & 0x3F))
    : static_cast<uint8_t>(0x80 | ((Code >> 6) & 0x3F));
  static constexpr uint8_t b3 =
    (length <= 3) ? 0
    : static_cast<uint8_t>(0x80 | (Code & 0x3F));
};

template<typename Pack>
struct PackToIn;
template<typename List1, typename List2, typename List3, typename List4>
struct PackToIn<ByteListPack<List1, List2, List3, List4>> {
  using type = In<List1, List2, List3, List4>;
};

template<typename Pack>
struct PackToExcept;
template<typename List1, typename List2, typename List3, typename List4>
struct PackToExcept<ByteListPack<List1, List2, List3, List4>> {
  using type = Except<List1, List2, List3, List4>;
};

template<typename CodePointList, typename Pack>
struct BuildInPack;
template<typename Pack>
struct BuildInPack<TypeList<>, Pack> {
  using type = typename PackToIn<Pack>::type;
};
template<typename List1, typename List2, typename List3, typename List4, typename Head, typename... Tails>
struct BuildInPack<TypeList<Head, Tails...>, ByteListPack<List1, List2, List3, List4>> {
  using Enc = Utf8Encoding<Head::value>;
  using NextPack = std::conditional_t<
    Enc::length == 1,
    ByteListPack<
      typename PushBackUnique<List1, Char<Enc::b0>>::type,
      List2, List3, List4
    >,
    std::conditional_t<
      Enc::length == 2,
      ByteListPack<
        typename PushBackUnique<List1, Char<Enc::b0>>::type,
        List2,
        List3,
        typename PushBackUnique<List4, Char<Enc::b1>>::type
      >,
      std::conditional_t<
        Enc::length == 3,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          List2,
          typename PushBackUnique<List3, Char<Enc::b1>>::type,
          typename PushBackUnique<List4, Char<Enc::b2>>::type
        >,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          typename PushBackUnique<List2, Char<Enc::b1>>::type,
          typename PushBackUnique<List3, Char<Enc::b2>>::type,
          typename PushBackUnique<List4, Char<Enc::b3>>::type
        >
      >
    >
  >;
  using type = typename BuildInPack<TypeList<Tails...>, NextPack>::type;
};

template<typename CodePointList, typename Pack>
struct BuildExceptPack;
template<typename Pack>
struct BuildExceptPack<TypeList<>, Pack> {
  using type = typename PackToExcept<Pack>::type;
};
template<typename List1, typename List2, typename List3, typename List4, typename Head, typename... Tails>
struct BuildExceptPack<TypeList<Head, Tails...>, ByteListPack<List1, List2, List3, List4>> {
  using Enc = Utf8Encoding<Head::value>;
  using NextPack = std::conditional_t<
    Enc::length == 1,
    ByteListPack<
      typename PushBackUnique<List1, Char<Enc::b0>>::type,
      List2, List3, List4
    >,
    std::conditional_t<
      Enc::length == 2,
      ByteListPack<
        typename PushBackUnique<List1, Char<Enc::b0>>::type,
        List2,
        List3,
        typename PushBackUnique<List4, Char<Enc::b1>>::type
      >,
      std::conditional_t<
        Enc::length == 3,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          List2,
          typename PushBackUnique<List3, Char<Enc::b1>>::type,
          typename PushBackUnique<List4, Char<Enc::b2>>::type
        >,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          typename PushBackUnique<List2, Char<Enc::b1>>::type,
          typename PushBackUnique<List3, Char<Enc::b2>>::type,
          typename PushBackUnique<List4, Char<Enc::b3>>::type
        >
      >
    >
  >;
  using type = typename BuildExceptPack<TypeList<Tails...>, NextPack>::type;
};

template<typename CodePointList>
struct BuildIn {
  using type = typename BuildInPack<
    CodePointList,
    ByteListPack<TypeList<>, TypeList<>, TypeList<>, TypeList<>>
  >::type;
};

template<typename CodePointList>
struct BuildExcept {
  using type = typename BuildExceptPack<
    CodePointList,
    ByteListPack<TypeList<>, TypeList<>, TypeList<>, TypeList<>>
  >::type;
};

/* ParseCharSetAtom: [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | Escape */
template<FixedString Pattern, size_t Pos>
struct ParseCharSetAtom {
  static_assert(Pos < Pattern.length, "ParseCharSetAtom: unexpected pattern ending");
  static_assert(is_in_class_char(Pattern[Pos]), "ParseCharSetAtom: unknown character");

  template<typename CodePointList, typename Pack>
  struct BuildInPack;
  template<typename List1, typename List2, typename List3, typename List4>
  struct BuildInPack<TypeList<>, ByteListPack<List1, List2, List3, List4>> {
    using type = In<List1, List2, List3, List4>;
  };
  template<typename List1, typename List2, typename List3, typename List4, typename Head, typename... Tails>
  struct BuildInPack<TypeList<Head, Tails...>, ByteListPack<List1, List2, List3, List4>> {
    using Enc = Utf8Encoding<Head::value>;
    using NextPack = std::conditional_t<
      Enc::length == 1,
      ByteListPack<
        typename PushBackUnique<List1, Char<Enc::b0>>::type,
        List2, List3, List4
      >,
      std::conditional_t<
        Enc::length == 2,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          List2,
          List3,
          typename PushBackUnique<List4, Char<Enc::b1>>::type
        >,
        std::conditional_t<
          Enc::length == 3,
          ByteListPack<
            typename PushBackUnique<List1, Char<Enc::b0>>::type,
            List2,
            typename PushBackUnique<List3, Char<Enc::b1>>::type,
            typename PushBackUnique<List4, Char<Enc::b2>>::type
          >,
          ByteListPack<
            typename PushBackUnique<List1, Char<Enc::b0>>::type,
            typename PushBackUnique<List2, Char<Enc::b1>>::type,
            typename PushBackUnique<List3, Char<Enc::b2>>::type,
            typename PushBackUnique<List4, Char<Enc::b3>>::type
          >
        >
      >
    >;
    using type = typename BuildInPack<TypeList<Tails...>, NextPack>::type;
  };

  template<typename CodePointList, typename Pack>
  struct BuildExceptPack;
  template<typename List1, typename List2, typename List3, typename List4>
  struct BuildExceptPack<TypeList<>, ByteListPack<List1, List2, List3, List4>> {
    using type = Except<List1, List2, List3, List4>;
  };
  template<typename List1, typename List2, typename List3, typename List4, typename Head, typename... Tails>
  struct BuildExceptPack<TypeList<Head, Tails...>, ByteListPack<List1, List2, List3, List4>> {
    using Enc = Utf8Encoding<Head::value>;
    using NextPack = std::conditional_t<
      Enc::length == 1,
      ByteListPack<
        typename PushBackUnique<List1, Char<Enc::b0>>::type,
        List2, List3, List4
      >,
      std::conditional_t<
        Enc::length == 2,
        ByteListPack<
          typename PushBackUnique<List1, Char<Enc::b0>>::type,
          List2,
          List3,
          typename PushBackUnique<List4, Char<Enc::b1>>::type
        >,
        std::conditional_t<
          Enc::length == 3,
          ByteListPack<
            typename PushBackUnique<List1, Char<Enc::b0>>::type,
            List2,
            typename PushBackUnique<List3, Char<Enc::b1>>::type,
            typename PushBackUnique<List4, Char<Enc::b2>>::type
          >,
          ByteListPack<
            typename PushBackUnique<List1, Char<Enc::b0>>::type,
            typename PushBackUnique<List2, Char<Enc::b1>>::type,
            typename PushBackUnique<List3, Char<Enc::b2>>::type,
            typename PushBackUnique<List4, Char<Enc::b3>>::type
          >
        >
      >
    >;
    using type = typename BuildExceptPack<TypeList<Tails...>, NextPack>::type;
  };

  struct impl_char {
    using CodePointParse = ParseUtf8CodePoint<Pattern, Pos>;
    using type = typename BuildCodePointRange<CodePointParse::value, CodePointParse::value>::type;
    static constexpr size_t next = CodePointParse::next;
  };

  struct impl_seq {
    using Start = ParseUtf8CodePoint<Pattern, Pos>;
    static_assert(Start::next < Pattern.length && Pattern[Start::next] == '-',
      "ParseCharSetAtom: `-` has no ending");
    using End = ParseUtf8CodePoint<Pattern, Start::next + 1>;
    using type = typename BuildCodePointRange<Start::value, End::value>::type;
    static constexpr size_t next = End::next;
  };

  struct impl_escape {
    static_assert(Pos + 1 < Pattern.length, "unexpected ending"); 
    static_assert(
      Pattern[Pos + 1] != 'w' && Pattern[Pos + 1] != 'W' && Pattern[Pos + 1] != 'd' 
      && Pattern[Pos + 1] != 'D' && Pattern[Pos + 1] != 's' && Pattern[Pos + 1] != 'S', 
      "char set do not support \\w, \\W, \\d, \\D, \\s, \\S. please use ([...]|\\w) explicitly"
    );
    using EscapeParse = ParseEscape<Pattern, Pos>;
    using type = TypeList<CodePoint<EscapeParse::code>>;
    static constexpr size_t next = EscapeParse::next;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  static constexpr bool has_hyphen = !is_escape
    && ParseUtf8CodePoint<Pattern, Pos>::next < Pattern.length
    && Pattern[ParseUtf8CodePoint<Pattern, Pos>::next] == '-';
  using chosen = std::conditional_t<
    is_escape,
    impl_escape,
    std::conditional_t<has_hyphen, impl_seq, impl_char>
  >;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharSet: CharSetAtom CharSet | CharSetAtom */
template<FixedString Pattern, size_t Pos>
struct ParseCharSet {
  using CharSetAtom = ParseCharSetAtom<Pattern, Pos>;
  static_assert(CharSetAtom::next <= Pattern.length,
    "ParseCharSet: char set atom parsing overflow");

  struct impl_run_on {
    using Next = ParseCharSet<Pattern, CharSetAtom::next>;
    static_assert(Next::next <= Pattern.length, "ParseCharSet: next parsing overflow");
    using type = typename JoinUnique<typename CharSetAtom::type, typename Next::type>::type;
    static constexpr size_t next = Next::next;
  };

  struct impl_stop {
    using type = typename CharSetAtom::type;
    static constexpr size_t next = CharSetAtom::next;
  };

  static constexpr bool run_on = CharSetAtom::next < Pattern.length
    && is_in_class_char(Pattern[CharSetAtom::next]);
  using chosen = std::conditional_t<run_on, impl_run_on, impl_stop>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharGroup: '[' CharSet ']' | '[' '^' CharSet ']' */
template<FixedString Pattern, size_t Pos>
struct ParseCharGroup {
  struct impl_pos {
    using CharSet = ParseCharSet<Pattern, Pos + 1>;
    static_assert(CharSet::next <= Pattern.length, "ParseCharGroup: char set parsing overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']',
      "ParseCharGroup: ']' not closed");
    using type = typename BuildIn<typename CharSet::type>::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  struct impl_neg {
    using CharSet = ParseCharSet<Pattern, Pos + 2>;
    static_assert(CharSet::next <= Pattern.length, "ParseCharGroup: char set parsing overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']',
      "ParseCharGroup: ']' not closed");
    using type = typename BuildExcept<typename CharSet::type>::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  static constexpr bool is_neg = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '^';
  using chosen = std::conditional_t<is_neg, impl_neg, impl_pos>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

template<FixedString Pattern, size_t Pos>
struct ParseComment {
  static_assert(Pos < Pattern.length, "comment not close");

  struct impl_close {
    static constexpr size_t next = Pos + 1;
  };
  struct impl_run_on {
    static constexpr size_t next = ParseComment<Pattern, Pos + 1>::next;
  };

  static constexpr size_t next = std::conditional<Pattern[Pos] == ')', impl_close, impl_run_on>::type::next;
};

/* ParseAtom: '(' ('?:')? Regex ')' | '[' CharSet ']' |  CHAR | '.' | '(?#' comment ')' */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseAtom {
  static_assert(
    Pos < Pattern.length && (
      Pattern[Pos] == '(' || Pattern[Pos] == '[' || Pattern[Pos] == '.'
      || Pattern[Pos] == '\\' || is_valid_char(Pattern[Pos])
    ), "ParseAtom: unknown character"
  );

  /* case '(' Regex ')' or '(?:' Regex ')' or '(?#' comment ')' */
  struct impl_paren {
    static constexpr bool is_comment =
      Pos + 2 < Pattern.length && Pattern[Pos + 1] == '?' && Pattern[Pos + 2] == '#';

    struct impl_regular {
      static constexpr bool is_non_capturing =
        Pos + 2 < Pattern.length && Pattern[Pos + 1] == '?' && Pattern[Pos + 2] == ':';
      using Regex = ParseRegex<Pattern, Pos + (is_non_capturing ? 3 : 1), CapIdx + (is_non_capturing ? 0 : 1)>;
      static_assert(Regex::next <= Pattern.length, "ParseAtom: regex parse overflow");
      static_assert(Regex::next < Pattern.length && Pattern[Regex::next] == ')',
        "ParseAtom impl_paren: missing closing ')' in pattern");
      using type = std::conditional_t<
        is_non_capturing,
        typename Regex::type,
        Concat<
          SetSlot<2 * CapIdx>,
          Concat<typename Regex::type, SetSlot<2 * CapIdx + 1>>
        >
      >;
      static constexpr size_t next = Regex::next + 1;
      static constexpr size_t next_cap_idx = Regex::next_cap_idx;
    };

    struct impl_comment {
      using type = Epsilon;
      static constexpr size_t next = ParseComment<Pattern, Pos>::next;
      static constexpr size_t next_cap_idx = CapIdx;
    };

    using chosen = typename std::conditional<is_comment, impl_comment, impl_regular>::type;
    using type = typename chosen::type;
    static constexpr size_t next = chosen::next;
    static constexpr size_t next_cap_idx = chosen::next_cap_idx;
  };

  /* case CharGroup */
  struct impl_square {
    using CharGroup = ParseCharGroup<Pattern, Pos>;
    static_assert(CharGroup::next <= Pattern.length, "ParseAtom: char set parse overflow");
    using type = typename CharGroup::type;
    static constexpr size_t next = CharGroup::next;
    static constexpr size_t next_cap_idx = CapIdx;
  };

  /* case CHAR */
  struct impl_char {
    using CHAR = ParseCHAR<Pattern, Pos>;
    static_assert(CHAR::next <= Pattern.length, "ParseAtom: char parse overflow");
    using type = typename CHAR::type;
    static constexpr size_t next = CHAR::next;
    static constexpr size_t next_cap_idx = CapIdx;
  };

  /* case '.' */
  struct impl_full_match {
    using type = Wildcard;
    static constexpr size_t next = Pos + 1;
    static constexpr size_t next_cap_idx = CapIdx;
  };

  using chosen = std::conditional_t<
    Pattern[Pos] == '(',
    impl_paren,
    std::conditional_t<
      Pattern[Pos] == '[',
      impl_square,
      std::conditional_t<
        Pattern[Pos] == '.',
        impl_full_match,
        impl_char
      >
    >
  >;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

template<typename AtomType, int64_t Min, int64_t Max>
struct BuildQuantifier {
  using type = Concat<AtomType, typename BuildQuantifier<AtomType, Min - 1, Max - 1>::type>;
};
template<typename AtomType, int64_t Max>
struct BuildQuantifier<AtomType, 0, Max> {
  struct inf {
    using type = Closure<AtomType>;
  };
  struct non_inf {
    using type = Or<
      Epsilon,
      Concat<AtomType, typename BuildQuantifier<AtomType, 0, Max - 1>::type>
    >;
  };
  using type = typename std::conditional_t<Max < 0, inf, non_inf>::type;
};
template<typename AtomType>
struct BuildQuantifier<AtomType, 0, 0> {
  using type = Epsilon;
};

/* ParseFactor: Atom ('*')? | Atom ('+')? | Atom ('?')?
 *            | Atom '{' Number ',' '}' | Atom '{' Number ',' Number '}'
 *            | Atom '{' ',' Number '}' | '{' Number '}'
 */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseFactor {
  using Atom = ParseAtom<Pattern, Pos, CapIdx>;

  static_assert(Atom::next <= Pattern.length, "ParseFactor: atom parse overflow");

  struct star {
    using type = Closure<typename Atom::type>;
    static constexpr size_t next = Atom::next + 1;
  };

  struct plus {
    using type = Concat<typename Atom::type, Closure<typename Atom::type>>;
    static constexpr size_t next = Atom::next + 1;
  };

  struct question {
    using type = Or<Epsilon, typename Atom::type>;
    static constexpr size_t next = Atom::next + 1;
  };

  struct curly {
    static_assert(Atom::next + 1 < Pattern.length, "ParseFactor: incomplete quantifier");

    static_assert(Pattern[Atom::next + 1] != '}', "ParseFactor: empty quantifier '{}' is invalid");

    using ParseMin = ParseDecimal<Pattern, Atom::next + 1>;
    static constexpr bool min_missing = (Pattern[Atom::next + 1] == ',');
    static constexpr int64_t Min = min_missing ? 0 : ParseMin::value;

    static_assert(ParseMin::next < Pattern.length
      && (Pattern[ParseMin::next] == ',' || Pattern[ParseMin::next] == '}'),
      "ParseFactor: incomplete quantifier");

    struct single_num {
      static constexpr int64_t Max = Min;
      static constexpr size_t next = ParseMin::next + 1;
    };

    struct multiple_num {
      static_assert(ParseMin::next + 1 < Pattern.length, "ParseFactor: incomplete quantifier");

      static_assert(!(min_missing && Pattern[ParseMin::next + 1] == '}'),
        "ParseFactor: quantifier must contain at least one number");

      using ParseMax = ParseDecimal<Pattern, ParseMin::next + 1>;
      static_assert(ParseMax::next < Pattern.length && Pattern[ParseMax::next] == '}',
        "ParseFactor: incomplete quantifier (missing '}')");

      static constexpr int64_t Max =
        (Pattern[ParseMin::next + 1] == '}') ? -1 : ParseMax::value;
      static constexpr size_t next = ParseMax::next + 1;
    };

    using chosen = std::conditional_t<Pattern[ParseMin::next] == '}', single_num, multiple_num>;
    static constexpr int64_t Max = chosen::Max;

    static_assert(Max < 0 || Min <= Max, "ParseFactor: invalid quantifier");

    using type = typename BuildQuantifier<typename Atom::type, Min, Max>::type;
    static constexpr size_t next = chosen::next;
  };

  static constexpr bool has_star = (Atom::next < Pattern.length && Pattern[Atom::next] == '*');
  static constexpr bool has_plus = (Atom::next < Pattern.length && Pattern[Atom::next] == '+');
  static constexpr bool has_question = (Atom::next < Pattern.length && Pattern[Atom::next] == '?');
  static constexpr bool has_curly = (Atom::next < Pattern.length && Pattern[Atom::next] == '{');

  using chosen = std::conditional_t<
    has_star,
    star,
    std::conditional_t<
      has_plus,
      plus,
      std::conditional_t<
        has_question,
        question,
        std::conditional_t<
          has_curly,
          curly,
          Atom
        >
      >
    >
  >;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = Atom::next_cap_idx;
};

/* ParseTerm := Factor Term | (empty -> Epsilon) */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseTerm {
  /* empty case */
  struct impl_empty {
    using type = Epsilon;
    static constexpr size_t next = Pos;
    static constexpr size_t next_cap_idx = CapIdx;
  };
  /* Factor Term case */
  struct impl_nonempty {
    using Factor = ParseFactor<Pattern, Pos, CapIdx>;
    static_assert(Factor::next <= Pattern.length, "ParseTerm: factor parse overflow");
    using Term = ParseTerm<Pattern, Factor::next, Factor::next_cap_idx>;
    static_assert(Term::next <= Pattern.length, "ParseTerm: term parse overflow");
    using type = Concat<typename Factor::type, typename Term::type>;
    static constexpr size_t next = Term::next;
    static constexpr size_t next_cap_idx = Term::next_cap_idx;
  };

  using chosen = std::conditional_t<
    (Pos >= Pattern.length) || (Pattern[Pos] == '|') || (Pattern[Pos] == ')'),
    impl_empty,
    impl_nonempty
  >;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

/* ParseRegex := Term ('|' Regex)? */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseRegex {
  using Term = ParseTerm<Pattern, Pos, CapIdx>;

  static_assert(Term::next <= Pattern.length, "ParseRegex: term parse overflow");

  struct impl_bar {
    using Regex = ParseRegex<Pattern, Term::next + 1, Term::next_cap_idx>;
    static_assert(Regex::next <= Pattern.length, "ParseRegex: regex parse overflow");
    using type = Or<typename Term::type, typename Regex::type>;
    static constexpr size_t next = Regex::next;
    static constexpr size_t next_cap_idx = Regex::next_cap_idx;
  };
  struct impl_no_bar {
    using type = typename Term::type;
    static constexpr size_t next = Term::next;
    static constexpr size_t next_cap_idx = Term::next_cap_idx;
  };

  static constexpr bool has_bar = (Term::next < Pattern.length && Pattern[Term::next] == '|');
  using chosen = std::conditional_t<has_bar, impl_bar, impl_no_bar>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

template<FixedString Pattern>
struct RegexScan {
  using Parse = ParseRegex<Pattern, 0, 1>;
  using type = typename Simplify<
    Concat<SetSlot<0>, Concat<typename Parse::type, SetSlot<1>>>
  >::type;
  static_assert(Parse::next == Pattern.length,
    "RegexScan: pattern not fully consumed or contains unexpected trailing characters");
};

} /* namespace impl */
} /* namespace onre */

// === snippet end ===

#endif