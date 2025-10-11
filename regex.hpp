#ifndef ONRE_REGEX_HPP
#define ONRE_REGEX_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include <cstdio>

namespace onre {
namespace impl {

/* === type list, a linear container of types === */
template<typename... Ts> struct TypeList {
  template<typename T>
  static constexpr bool Contains = (std::is_same_v<T, Ts> || ...);

  template<typename T>
  static constexpr std::size_t IndexOf = []{
    std::size_t index = 0;
    bool found = false;
    ([&]<typename U>(std::type_identity<U>) {
      if (!found && std::is_same_v<T, U>) {
        found = true;
        return;
      }
      if (!found) ++index;
    }(std::type_identity<Ts>{}), ...);
    return found ? index : static_cast<std::size_t>(-1);
  }();

  static constexpr bool length = sizeof...(Ts);
};

template<typename List, typename T>
struct PushBackUnique;
template<typename... Ts, typename T>
struct PushBackUnique<TypeList<Ts...>, T> {
  using type = std::conditional_t<
    TypeList<Ts...>::template Contains<T>,
    TypeList<Ts...>,
    TypeList<Ts..., T>
  >;
};

template<typename List1, typename List2>
struct JoinUnique;
template<typename List1, typename Head, typename... Tail>
struct JoinUnique<List1, TypeList<Head, Tail...>> {
  using TmpAcc = typename PushBackUnique<List1, Head>::type;
  using type = typename JoinUnique<TmpAcc, TypeList<Tail...>>::type;
};
template<typename List1>
struct JoinUnique<List1, TypeList<>> {
  using type = List1;
};

template<template<typename> typename Func, typename List> struct Map;
template<template<typename> typename Func, typename... Ts>
struct Map<Func, TypeList<Ts...>> {
  using type = TypeList<typename Func<Ts>::type...>;
};

template<template<typename> typename IsKeep, typename List, typename Acc> struct FilterImpl;
template<template<typename> typename IsKeep, typename Head, typename... Tails, typename Acc>
struct FilterImpl<IsKeep, TypeList<Head, Tails...>, Acc> {
  using type = std::conditional_t<
    IsKeep<Head>::value,
    FilterImpl<IsKeep, TypeList<Tails...>, typename PushBackUnique<Acc, Head>::type>,
    FilterImpl<IsKeep, TypeList<Tails...>, Acc>
  >::type;
};
template<template<typename> typename IsKeep, typename Acc>
struct FilterImpl<IsKeep, TypeList<>, Acc> {
  using type = Acc;
};
template<template<typename> typename IsKeep, typename List> struct Filter {
  using type = FilterImpl<IsKeep, List, TypeList<>>::type;
};

template<template<typename, typename> typename MergeFunc, typename List, typename Begin> struct RightFold;
template<template<typename, typename> typename MergeFunc, typename Head, typename... Tails, typename Begin>
struct RightFold<MergeFunc, TypeList<Head, Tails...>, Begin> {
  using type = typename MergeFunc<Head, typename RightFold<MergeFunc, TypeList<Tails...>, Begin>::type>::type;
};
template<template<typename, typename> typename MergeFunc, typename Begin>
struct RightFold<MergeFunc, TypeList<>, Begin> {
  using type = Begin;
};

template<template<typename, typename> typename MergeFunc, typename List, typename Acc> struct LeftFoldImpl;
template<template<typename, typename> typename MergeFunc, typename Head, typename... Tails, typename Acc>
struct LeftFoldImpl<MergeFunc, TypeList<Head, Tails...>, Acc> {
  using TmpAcc = MergeFunc<Acc, Head>::type;
  using type = LeftFoldImpl<MergeFunc, TypeList<Tails...>, TmpAcc>::type;
};
template<template<typename, typename> typename MergeFunc, typename Acc>
struct LeftFoldImpl<MergeFunc, TypeList<>, Acc> {
  using type = Acc;
};

template<template<typename, typename> typename MergeFunc, typename List, typename Begin> struct LeftFold {
  using type = LeftFoldImpl<MergeFunc, List, Begin>::type;
};


/* === fixed string, a string container enabling compile-time visiting === */
template<size_t N>
struct FixedString {
  consteval FixedString(const char (&str)[N]) {
    std::copy_n(str, N, data.begin());
  }
  FixedString(const FixedString&) = delete;

  std::array<char, N> data{};

  static constexpr size_t length = N - 1;
  constexpr const char* c_str() const { return data.data(); }
  constexpr char operator[](size_t i) const { return data[i]; }
};
template<size_t N>
FixedString(const char (&str)[N]) -> FixedString<N>;


/* === extended regular expression tree representation with zero-width action === */
struct EmptySet {};
struct Epsilon {};
template<char C> struct Char { static constexpr char c = C; };
template<std::size_t I> struct SetSlot { static constexpr char i = I; };
template<typename R, typename S> struct Or { using left = R; using right = S; };
template<typename R, typename S> struct Concat {using left = R; using right = S; };
template<typename R> struct Closure { using inner = R; };

template<typename R> struct is_empty_set : std::is_same<R, EmptySet> {};
template<typename R> struct is_epsilon : std::is_same<R, Epsilon> {};
template<typename R> struct is_char : std::false_type {};
template<char C> struct is_char<Char<C>> : std::true_type {};
template<typename R> struct is_setslot : std::false_type {};
template<size_t I> struct is_setslot<SetSlot<I>> : std::true_type {};
template<typename> struct is_or : std::false_type {};
template<typename L, typename R> struct is_or<Or<L, R>> : std::true_type {};
template<typename> struct is_concat : std::false_type {};
template<typename L, typename R> struct is_concat<Concat<L, R>> : std::true_type {};
template<typename> struct is_closure : std::false_type {};
template<typename T> struct is_closure<Closure<T>> : std::true_type {};

template<typename T> static constexpr auto is_empty_set_v = is_empty_set<T>::value;
template<typename T> static constexpr auto is_epsilon_v = is_epsilon<T>::value;
template<typename T> static constexpr auto is_char_v = is_char<T>::value;
template<typename T> static constexpr auto is_setslot_v = is_setslot<T>::value;
template<typename T> static constexpr auto is_or_v = is_or<T>::value;
template<typename T> static constexpr auto is_concat_v = is_concat<T>::value;
template<typename T> static constexpr auto is_closure_v = is_closure<T>::value;


/* === nullable testing, testing whether epsilon in L(R) === */
template<typename R> struct Nullable {};
template<> struct Nullable<EmptySet> : std::false_type {};
template<> struct Nullable<Epsilon> : std::true_type {};
template<char C> struct Nullable<Char<C>> : std::false_type {};
template<size_t I> struct Nullable<SetSlot<I>> : std::true_type {};
template<typename L, typename R> struct Nullable<Or<L, R>> : std::bool_constant<Nullable<L>::value || Nullable<R>::value> {};
template<typename L, typename R> struct Nullable<Concat<L, R>> : std::bool_constant<Nullable<L>::value && Nullable<R>::value> {};
template<typename R> struct Nullable<Closure<R>> : std::true_type {};


/* === First notation, get TypeList<Char...> of possible character occurring in the head of string in RE language === */
template <typename RE, typename Acc>
struct First;
template <typename Acc> struct First<EmptySet, Acc> { using type = Acc; };
template <typename Acc> struct First<Epsilon, Acc> { using type = Acc; };
template <char C, typename Acc>
struct First<Char<C>, Acc> {
  using type = typename PushBackUnique<Acc, Char<C>>::type;
};
template <size_t I, typename Acc> struct First<SetSlot<I>, Acc> { using type = Acc; };
template <typename R, typename S, typename Acc>
struct First<Or<R, S>, Acc> {
  using TmpAcc = typename First<R, Acc>::type;
  using type = typename First<S, TmpAcc>::type;
};
template <typename R, typename S, typename Acc>
struct First<Concat<R, S>, Acc> {
  struct impl_r_nullable {
    using TmpAcc = typename First<R, Acc>::type;
    using type = typename First<S, TmpAcc>::type;
  };
  struct impl_r_non_nullable {
    using type = typename First<R, Acc>::type;
  };
  using type = typename std::conditional_t<Nullable<R>::value, impl_r_nullable, impl_r_non_nullable>::type;
};
template <typename R, typename Acc>
struct First<Closure<R>, Acc> {
  using type = typename First<R, Acc>::type;
};


/* === simplify and standard ordering rules, significantly reduce complexity === */
template<typename R> struct Simplify { using type = R; };
/* 0|R <=> R|0 <=> R */
template<typename R> struct Simplify<Or<EmptySet, R>> { using type = typename Simplify<R>::type; };
template<typename R> struct Simplify<Or<R, EmptySet>> { using type = typename Simplify<R>::type; };
/* R|R <=> R */
template<typename T> struct Simplify<Or<T, T>> { using type = typename Simplify<T>::type; };
template<> struct Simplify<Or<EmptySet, EmptySet>> { using type = EmptySet; };

/* 0R <=> R0 <=> 0 */
template<typename R> struct Simplify<Concat<EmptySet, R>> { using type = EmptySet; };
template<typename L> struct Simplify<Concat<L, EmptySet>> { using type = EmptySet; };
template<> struct Simplify<Concat<EmptySet, EmptySet>> { using type = EmptySet; };
/* eR <=> Re <=> R */
template<typename R> struct Simplify<Concat<Epsilon, R>> { using type = typename Simplify<R>::type; };
template<typename L> struct Simplify<Concat<L, Epsilon>> { using type = typename Simplify<L>::type; };
template<> struct Simplify<Concat<Epsilon, EmptySet>> { using type = EmptySet; };
template<> struct Simplify<Concat<EmptySet, Epsilon>> { using type = EmptySet; };
template<> struct Simplify<Concat<Epsilon, Epsilon>> { using type = Epsilon; };

/* 0* <=> e* <=> e */
template<> struct Simplify<Closure<EmptySet>> { using type = Epsilon; };
template<> struct Simplify<Closure<Epsilon>> { using type = Epsilon; };

/* R** <=> R* */
template<typename R> struct Simplify<Closure<Closure<R>>> { using type = typename Simplify<Closure<R>>::type; };

/* (e|R)* <=> (R|e)* <=> R* */
template<typename R> struct Simplify<Closure<Or<Epsilon, R>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Closure<Or<R, Epsilon>>> { using type = typename Simplify<Closure<R>>::type; };
template<> struct Simplify<Closure<Or<Epsilon, Epsilon>>> { using type = Epsilon; };
/* e|RR* <=> RR*|e <=> R* */
template<typename R> struct Simplify<Or<Epsilon, Concat<R, Closure<R>>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Or<Concat<R, Closure<R>>, Epsilon>> { using type = typename Simplify<Closure<R>>::type; };
/* RR*|R* <=> R*R|R <=> R*|RR* <=> R*|R*R <=> RR* */
template<typename R> struct Simplify<Or<Concat<R, Closure<R>>, Closure<R>>> { using type = typename Simplify<Concat<R, Closure<R>>>::type; };
template<typename R> struct Simplify<Or<Concat<Closure<R>, R>, Closure<R>>> { using type = typename Simplify<Concat<R, Closure<R>>>::type; };
template<typename R> struct Simplify<Or<Closure<R>, Concat<R, Closure<R>>>> { using type = typename Simplify<Concat<R, Closure<R>>>::type; };
template<typename R> struct Simplify<Or<Closure<R>, Concat<Closure<R>, R>>> { using type = typename Simplify<Concat<R, Closure<R>>>::type; };

/* standard ordering */
template<typename A, typename B>
struct is_less : std::false_type {};

/* Or < Concat < Closure < EmptySet < Epsilon < Char < SetSlot */
template <typename R1, typename S1, typename R2, typename S2> struct is_less<Or<R1, S1>, Concat<R2, S2>> : std::true_type {};
template <typename R1, typename S1, typename R2> struct is_less<Or<R1, S1>, Closure<R2>> : std::true_type {};
template <typename R, typename S> struct is_less<Or<R, S>, EmptySet> : std::true_type {};
template <typename R, typename S> struct is_less<Or<R, S>, Epsilon> : std::true_type {};
template <typename R, typename S, char C> struct is_less<Or<R, S>, Char<C>> : std::true_type {};
template <typename R, typename S, size_t I> struct is_less<Or<R, S>, SetSlot<I>> : std::true_type {};
template <typename R1, typename S1, typename R2> struct is_less<Concat<R1, S1>, Closure<R2>> : std::true_type {};
template <typename R, typename S> struct is_less<Concat<R, S>, EmptySet> : std::true_type {};
template <typename R, typename S> struct is_less<Concat<R, S>, Epsilon> : std::true_type {};
template <typename R, typename S, char C> struct is_less<Concat<R, S>, Char<C>> : std::true_type {};
template <typename R, typename S, size_t I> struct is_less<Concat<R, S>, SetSlot<I>> : std::true_type {};
template <typename R> struct is_less<Closure<R>, EmptySet> : std::true_type {};
template <typename R> struct is_less<Closure<R>, Epsilon> : std::true_type {};
template <typename R, char C> struct is_less<Closure<R>, Char<C>> : std::true_type {};
template <typename R, size_t I> struct is_less<Closure<R>, SetSlot<I>> : std::true_type {};
template <> struct is_less<EmptySet, Epsilon> : std::true_type {};
template <char C> struct is_less<EmptySet, Char<C>> : std::true_type {};
template <size_t I> struct is_less<EmptySet, SetSlot<I>> : std::true_type {};
template <char C> struct is_less<Epsilon, Char<C>> : std::true_type {};
template <size_t I> struct is_less<Epsilon, SetSlot<I>> : std::true_type {};
template <char C, size_t I> struct is_less<Char<C>, SetSlot<I>> : std::true_type {};

template <char C1, char C2>
struct is_less<Char<C1>, Char<C2>> { static constexpr bool value = C1 < C2; };
template <size_t I1, size_t I2>
struct is_less<SetSlot<I1>, SetSlot<I2>> { static constexpr bool value = I1 < I2; };
template<typename R1, typename S1, typename R2, typename S2>
struct is_less<Or<R1, S1>, Or<R2, S2>> { static constexpr bool value = is_less<R1, R2>::value || (!is_less<R1, R2>::value && is_less<S1, S2>::value); };
template<typename R1, typename S1, typename R2, typename S2>
struct is_less<Concat<R1, S1>, Concat<R2, S2>> { static constexpr bool value = is_less<R1, R2>::value || (!is_less<R1, R2>::value && is_less<S1, S2>::value); };
template<typename R1, typename R2>
struct is_less<Closure<R1>, Closure<R2>> { static constexpr bool value = is_less<R1, R2>::value; };

template<typename R, typename S, typename T> struct Simplify<Or<Or<R, S>, T>> { using type = typename Simplify<Or<R, Or<S, T>>>::type; };
template<typename R, typename S> struct Simplify<Or<Or<R, S>, Or<R, S>>> { using type = typename Simplify<Or<R, S>>::type; };
template<typename R, typename S> struct Simplify<Or<Or<R, S>, EmptySet>> { using type = typename Simplify<Or<R, S>>::type; };
template<typename R, typename S, typename T> struct Simplify<Concat<Concat<R, S>, T>> { using type = typename Simplify<Concat<R, Concat<S, T>>>::type; };
template<typename R, typename S> struct Simplify<Concat<Concat<R, S>, EmptySet>> { using type = EmptySet; };
template<typename R, typename S> struct Simplify<Concat<Concat<R, S>, Epsilon>> { using type = typename Simplify<Concat<R, S>>::type; };
template<typename R> struct Simplify<Concat<Closure<R>, R>> { using type = typename Simplify<Concat<R, Closure<R>>>::type; };
template<typename R> struct Simplify<Concat<Closure<R>, Epsilon>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Concat<Closure<R>, EmptySet>> { using type = EmptySet; };
template<typename R, typename S>
struct Simplify<Concat<Closure<Or<R, S>>, Or<R, S>>> {
  using type = typename Simplify<Closure<Or<R, S>>>::type;
};

/* recursive */
template<typename L, typename R> struct Simplify<Or<L, R>> {
  using type = std::conditional_t<
    is_less<L, R>::value,
    Or<typename Simplify<L>::type, typename Simplify<R>::type>,
    Or<typename Simplify<R>::type, typename Simplify<L>::type>
  >;
};
template<typename L, typename R> struct Simplify<Concat<L, R>> { using type = Concat<typename Simplify<L>::type, typename Simplify<R>::type>; };
template<typename R> struct Simplify<Closure<R>> { using type = Closure<typename Simplify<R>::type>; };

template<typename Expr> using Simplify_t = Simplify<Expr>::type;


/* === compile-time alphabet and helper function === */
constexpr size_t nr_ascii_char = 128;
constexpr char visible_ascii_start = ' ';
constexpr char visible_ascii_end = '~';

constexpr bool is_visible_char(char ch) {
  return ch >= visible_ascii_start && ch <= visible_ascii_end; // ASCII visible characters
}

constexpr std::array<bool, nr_ascii_char> make_valid_table() {
  std::array<bool, nr_ascii_char> table {};
  for (int32_t i = 0; i < nr_ascii_char; i++) {
    table[i] = is_visible_char(i) && i != '|' && i != '*' && i != '+'
      && i != '?' && i != '(' && i != ')' && i != '[' && i != ']'
      && i != '.';
  }
  return table;
}

constexpr auto valid_table = make_valid_table();

constexpr bool is_valid_char(char ch) {
  return valid_table[static_cast<unsigned char>(ch)];
}

constexpr bool is_in_class_char(char ch) {
  return is_visible_char(ch) && ch != ']';
}

template <char Start, char End>
struct BuildCharList {
  static_assert(Start <= End, "invalid char range");

  template <typename Indices> struct Impl;
  template <size_t... Is> struct Impl<std::index_sequence<Is...>> {
    using type = TypeList<Char<Start + Is>...>;
  };

  using type = Impl<std::make_index_sequence<End - Start + 1>>::type;
};

using Alphabet = typename BuildCharList<visible_ascii_start, visible_ascii_end>::type;


/* === regex parser === */
/*
  Grammar:
    Regex       := Term ('|' Regex)?
    Term        := Factor Term | (empty)
    Factor      := Atom ('*')? | Atom ('+')? | Atom ('?')?
    Atom        := '(' Regex ')' | CharGroup | CHAR | '.'
    CharGroup   := '[' CharSet ']' | '[' '^' CharSet ']'
    CharSet     := CharSetAtom CharSet | CharSetAtom
    CharSetAtom := [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | '\' [VISIBLE CHAR]
    CHAR        := [VALID CHAR] | '\' [VISIBLE CHAR]
    Empty input -> Epsilon
*/

/* forward declarations */
template<FixedString Pattern, size_t Pos, size_t CapIdx> struct ParseRegex;
template<FixedString Pattern, size_t Pos, size_t CapIdx> struct ParseTerm;
template<FixedString Pattern, size_t Pos, size_t CapIdx> struct ParseFactor;
template<FixedString Pattern, size_t Pos, size_t CapIdx> struct ParseAtom;
template<FixedString Pattern, size_t Pos> struct ParseCharGroup;
template<FixedString Pattern, size_t Pos> struct ParseCharSet;
template<FixedString Pattern, size_t Pos> struct ParseCharSetAtom;
template<FixedString Pattern, size_t Pos> struct ParseCHAR;

/* ParseCHAR : [VALID CHAR] | '\' [VISIBLE CHAR] */
template <FixedString Pattern, size_t Pos>
struct ParseCHAR {
  static_assert(
    Pos < Pattern.length && (Pattern[Pos] == '\\' || is_valid_char(Pattern[Pos])),
    "ParseCHAR: unknown character"
  );

  struct impl_escape {
    static_assert(Pos + 1 < Pattern.length, "ParseAtom: cannot find escape character");
    static_assert(is_visible_char(Pattern[Pos + 1]), "ParseAtom: unknown character");
    using type = Char<Pattern[Pos + 1]>;
    static constexpr size_t next = Pos + 2;
  };

  struct impl_simple {
    using type = Char<Pattern[Pos]>;
    static constexpr size_t next = Pos + 1;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  using chosen = std::conditional_t<is_escape, impl_escape, impl_simple>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharSetAtom: [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | '\' [VISIBLE CHAR] */
template<FixedString Pattern, size_t Pos>
struct ParseCharSetAtom {
  static_assert(Pos < Pattern.length, "ParseCharSetAtom: unexpected pattern ending");
  static_assert(is_in_class_char(Pattern[Pos]), "ParseCharSetAtom: unknown character");

  struct impl_seq {
    static_assert(Pos + 2 < Pattern.length && is_in_class_char(Pattern[Pos + 2]), "ParseCharSetAtom: `-` has no ending");
    using type = typename BuildCharList<Pattern[Pos], Pattern[Pos + 2]>::type;
    static constexpr size_t next = Pos + 3;
  };

  struct impl_char {
    using type = TypeList<Char<Pattern[Pos]>>;
    static constexpr size_t next = Pos + 1;
  };

  struct impl_escape {
    static_assert(Pos + 1 < Pattern.length && is_visible_char(Pattern[Pos + 1]), "ParseCharSetAtom: cannot find escaped character");
      using type = TypeList<Char<Pattern[Pos + 1]>>;
    static constexpr size_t next = Pos + 2;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  static constexpr bool has_hyphen = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '-';
  using chosen = std::conditional_t<is_escape, impl_escape, std::conditional_t<has_hyphen, impl_seq, impl_char>>;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharSet: CharSetAtom CharSet | CharSetAtom */
template<FixedString Pattern, size_t Pos>
struct ParseCharSet {
  using CharSetAtom = ParseCharSetAtom<Pattern, Pos>;
  static_assert(CharSetAtom::next <= Pattern.length, "ParseCharSet: char set atom parsing overflow");

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

  static constexpr bool run_on = CharSetAtom::next < Pattern.length && is_in_class_char(Pattern[CharSetAtom::next]);
  using chosen = std::conditional_t<run_on, impl_run_on, impl_stop>;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

template<typename CharList>
struct CharListToOrSequential {
  template<typename X, typename Y> struct BuildOr { using type = Or<X, Y>; };
  using type = Simplify<typename RightFold<BuildOr, CharList, EmptySet>::type>::type;
};

template<typename CharList, typename Alphabet>
struct CharListNegation {
  template <typename Char>
  struct NotInList {
    static constexpr bool value = !CharList::template Contains<Char>;
  };
  using type = Filter<NotInList, Alphabet>::type;
};

/* ParseCharGroup: '[' CharSet ']' | '[' '^' CharSet ']' */
template<FixedString Pattern, size_t Pos>
struct ParseCharGroup {
  struct impl_pos {
    using CharSet = ParseCharSet<Pattern, Pos + 1>;
    static_assert(CharSet::next <= Pattern.length, "ParseCharGroup: char set parsing overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']', "ParseCharGroup: ']' not closed");
    using type = typename CharListToOrSequential<typename CharSet::type>::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  struct impl_neg {
    using CharSet = ParseCharSet<Pattern, Pos + 2>;
    static_assert(CharSet::next <= Pattern.length, "ParseCharGroup: char set parsing overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']', "ParseCharGroup: ']' not closed");
    using NegCharList = typename CharListNegation<typename CharSet::type, Alphabet>::type;
    using type = typename CharListToOrSequential<NegCharList>::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  static constexpr bool is_neg = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '^';
  using chosen = std::conditional_t<is_neg, impl_neg, impl_pos>;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

template<typename Alphabet> struct FullMatch {
  template<typename X, typename Y> struct BuildOr { using type = Or<X, Y>; };
  using type = Simplify<typename RightFold<BuildOr, Alphabet, EmptySet>::type>::type;
};

/* ParseAtom: '(' Regex ')' | '[' CharSet ']' |  CHAR | '.' */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseAtom {
  static_assert(
    Pos < Pattern.length && (
      Pattern[Pos] == '(' || Pattern[Pos] == '[' || Pattern[Pos] == '.'
      || Pattern[Pos] == '\\' || is_visible_char(Pattern[Pos])
    ), "ParseAtom: unknown character"
  );

  /* case '(' Regex ')' */
  struct impl_paren {
    using Regex = ParseRegex<Pattern, Pos + 1, CapIdx + 1>;
    static_assert(Regex::next <= Pattern.length, "ParseAtom: regex parse overflow");
    static_assert(Regex::next < Pattern.length && Pattern[Regex::next] == ')',
      "ParseAtom impl_paren: missing closing ')' in pattern");
    using type = Concat<SetSlot<2 * CapIdx>, Concat<typename Regex::type, SetSlot<2 * CapIdx + 1>>>;
    static constexpr size_t next = Regex::next + 1;
    static constexpr size_t next_cap_idx = Regex::next_cap_idx;
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
    using type = typename FullMatch<Alphabet>::type;
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
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

/* ParseFactor: Atom ('*')? | Atom ('+')? | Atom ('?')? */
template<FixedString Pattern, size_t Pos, size_t CapIdx>
struct ParseFactor {
  using Atom = ParseAtom<Pattern, Pos, CapIdx>;

  static_assert(Atom::next <= Pattern.length, "ParseFactor: atom parse overflow");

  static constexpr bool has_star = (Atom::next < Pattern.length && Pattern[Atom::next] == '*');
  static constexpr bool has_plus = (Atom::next < Pattern.length && Pattern[Atom::next] == '+');
  static constexpr bool has_question = (Atom::next < Pattern.length && Pattern[Atom::next] == '?');

  using type = typename Simplify<std::conditional_t<
    has_star,
    Closure<typename Atom::type>,
    std::conditional_t<
      has_plus,
      Concat<typename Atom::type, Closure<typename Atom::type>>,
      std::conditional_t<
        has_question,
        Or<Epsilon, typename Atom::type>,
        typename Atom::type
      >
    >
  >>::type;
  static constexpr size_t next = (has_star || has_plus || has_question) ? (Atom::next + 1) : Atom::next;
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
  using type = typename Simplify<typename chosen::type>::type;
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
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

template<FixedString Pattern>
struct RegexScan {
  using Parse = ParseRegex<Pattern, 0, 1>;
  using type = Concat<SetSlot<0>, Concat<typename Parse::type, SetSlot<1>>>;
  static_assert(Parse::next == Pattern.length, "RegexScan: pattern not fully consumed or contains unexpected trailing characters");
};

/* for fast O(|s|) bool matching with little O(1) */
namespace dfa {

/* === classic brzozowski derivative === */
template<typename R, char C> struct Derivative;
/* d0/dc = 0 */
template<char C> struct Derivative<EmptySet, C> { using type = EmptySet; };
/* de/dc = 0 */
template<char C> struct Derivative<Epsilon, C> { using type = EmptySet; };
/* dx/dc = x == c ? e : 0 */
template<char X, char C>
struct Derivative<Char<X>, C> {
  using type = std::conditional_t<X == C, Epsilon, EmptySet>;
};
/* d(R|S)/dc = dR/dc | dS/dc */
template<typename R, typename S, char C>
struct Derivative<Or<R, S>, C> {
  using type = typename Simplify<Or<typename Derivative<R, C>::type, typename Derivative<S, C>::type>>::type;
};
/* d(RS)/dc = dR/dc S | (delta(R) ? dS/dc : 0)*/
template<typename L, typename R, char C>
struct Derivative<Concat<L, R>, C> {
  using Part1 = Concat<typename Derivative<L, C>::type, R>;
  using Part2 = std::conditional_t<
    Nullable<L>::value,
    typename Derivative<R, C>::type,
    EmptySet
  >;
  using type = typename Simplify<Or<Part1, Part2>>::type;
};
/* d(R*)/dc = dR/dc R* */
template<typename R, char C>
struct Derivative<Closure<R>, C> {
    using type = typename Simplify<Concat<typename Derivative<R, C>::type, Closure<R>>>::type;
};


/* === DFA builder === */
template<typename R> struct RemoveAllAction;
template<> struct RemoveAllAction<EmptySet> { using type = EmptySet; };
template<> struct RemoveAllAction<Epsilon> { using type = Epsilon; };
template<char C> struct RemoveAllAction<Char<C>> { using type = Char<C>; };
template<size_t I> struct RemoveAllAction<SetSlot<I>> { using type = Epsilon; };
template<typename L, typename R> struct RemoveAllAction<Or<L, R>> {
  using type = typename Simplify<
    Or<typename RemoveAllAction<L>::type, typename RemoveAllAction<R>::type>
  >::type;
};
template<typename L, typename R> struct RemoveAllAction<Concat<L, R>> {
  using type = typename Simplify<
    Concat<typename RemoveAllAction<L>::type, typename RemoveAllAction<R>::type>
  >::type;
};
template<typename R> struct RemoveAllAction<Closure<R>> : std::true_type {
  using type = typename Simplify<
    Closure<typename RemoveAllAction<R>::type>
  >::type;
};

template<typename R>
struct State {
  using re = R;
  static constexpr bool accepting = Nullable<R>::value;
};

template<std::size_t From, char C, std::size_t To>
struct Edge { static constexpr std::size_t from = From; static constexpr char ch = C; static constexpr std::size_t to = To; };

template<typename StatesList, typename EdgesList>
struct StateEdgePair {
  using States = StatesList;
  using Edges = EdgesList;
};

/* Append derivative for a single character C, given current accumulator Pair<States,Edges> and a source StateT */
template<typename PairAcc, typename StateT, char C>
struct AppendDerivativeChar {
private:
  using current_states = typename PairAcc::States;
  using current_edges  = typename PairAcc::Edges;
  using RE_t = typename StateT::re;
  using Der_t = typename Derivative<RE_t, C>::type;

  /* If derivative is EmptySet -> no change */
  static constexpr bool is_empty = std::is_same<Der_t, EmptySet>::value;
public:
  using type = std::conditional_t<
    is_empty,
    PairAcc,
    StateEdgePair<
      typename PushBackUnique<current_states, State<Der_t>>::type,
      typename PushBackUnique<
        current_edges,
        Edge<
          PushBackUnique<current_states, State<Der_t>>::type::template IndexOf<StateT>,
          C,
          PushBackUnique<current_states, State<Der_t>>::type::template IndexOf<State<Der_t>>
        >
      >::type
    >
  >;
};

/* derive state for all character in given alphabet */
template<typename PairAcc, typename StateT, typename Alphabet>
struct AppendDerivativesForStatePair;
template<typename PairAcc, typename StateT>
struct AppendDerivativesForStatePair<PairAcc, StateT, TypeList<>> {
  using type = PairAcc;
};
template<typename PairAcc, typename StateT, typename Head, typename... Tail>
struct AppendDerivativesForStatePair<PairAcc, StateT, TypeList<Head, Tail...>> {
  using after_processing = typename AppendDerivativeChar<PairAcc, StateT, Head::c>::type;
  using type = typename AppendDerivativesForStatePair<after_processing, StateT, TypeList<Tail...>>::type;
};

/* expand once on a list of states, produce Pair<NewStates, NewEdges> */
template<typename StateEdgePair>
struct ExpandOnce {
private:
  /* fold over each state: for each state, append derivatives across alphabet */
  template<typename PairSoFar, typename StateT>
  struct DoState {
    using PossibleChars = typename First<typename StateT::re, TypeList<>>::type;
    using type = typename AppendDerivativesForStatePair<PairSoFar, StateT, PossibleChars>::type;
  };

  /* compile-time fold: iterate through States... */
  template<typename Acc, typename... Remaining> struct Fold;
  template<typename Acc, typename HeadState, typename... Rest>
  struct Fold<Acc, HeadState, Rest...> {
    using after = typename DoState<Acc, HeadState>::type;
    using type = typename Fold<after, Rest...>::type;
  };
  template<typename Acc>
  struct Fold<Acc> { using type = Acc; };

  /* unpacking List */
  template<typename Acc, typename StateList> struct FoldHelper;
  template<typename Acc, typename... States>
  struct FoldHelper<Acc, TypeList<States...>> {
      using type = typename Fold<Acc, States...>::type;
  };

public:
  using type = typename FoldHelper<StateEdgePair, typename StateEdgePair::States>::type;
};

/* expand until stable pair: iterate expand once until states length doesn't increase */
/* separate impl to avoid possible self reference causing errors */
template<typename PairAcc, std::size_t PrevLen>
struct ExpandUntilStablePairImpl;

template<typename PairAcc, std::size_t PrevLen>
struct ExpandUntilStablePairImpl {
private:
  using NextPair = typename ExpandOnce<PairAcc>::type;
  static constexpr std::size_t next_len = NextPair::States::length;

public:
  using type = typename std::conditional_t<
    next_len == PrevLen,
    std::type_identity<NextPair>,
    ExpandUntilStablePairImpl<NextPair, next_len>
  >::type;
};

template<typename PairAcc, std::size_t PrevLen = PairAcc::States::length>
struct ExpandUntilStablePair {
  using type = typename ExpandUntilStablePairImpl<PairAcc, PrevLen>::type;
};

template<typename RE>
struct AllStatesAndEdgesGenerator {
private:
  using initial_states = TypeList<State<RE>>;
  using initial_pair = StateEdgePair<initial_states, TypeList<>>;
public:
  using type = typename ExpandUntilStablePair<initial_pair, initial_states::length>::type;
  using States = typename type::States;
  using Edges  = typename type::Edges;
};

/* aliases */
template<typename RE> using AllStateEdgePair = typename AllStatesAndEdgesGenerator<RE>::type;
template<typename RE> using AllStatesList = typename AllStatesAndEdgesGenerator<RE>::States;
template<typename RE> using AllEdgesList  = typename AllStatesAndEdgesGenerator<RE>::Edges;


/* === table builder, convert sparse graph representation into jump table representation === */
template<typename EdgesList>
struct BuildTable {
  template<std::size_t N>
  static consteval void make() {
    static_assert(false, "impossible: should never fall through");
  }
};
template<typename... Edges>
struct BuildTable<TypeList<Edges...>> {
  template<std::size_t N>
  static consteval std::array<std::array<int32_t, nr_ascii_char>, N> make() {
    std::array<std::array<int32_t, nr_ascii_char>, N> table{};
    for (auto &row : table) row.fill(-1);
    ((table[Edges::from][static_cast<std::size_t>(Edges::ch)] = Edges::to), ...);
    return table;
  }
};

template<typename StatesList>
struct BuildAccepts;
template<typename... Ss>
struct BuildAccepts<TypeList<Ss...>> {
  static consteval std::array<bool, sizeof...(Ss)> make() {
    return std::array<bool, sizeof...(Ss)>{ Ss::accepting... };
  }
};

} /* namespace dfa */

/*
 * for slower O(|s| * #capture * 2^|pattern|) matching with capture group.
 * the matching time is still linear in the length of the input string,
 * but is related to the number of capture group (linear) and the length
 * of the pattern (in the worst case exponential), and have  a bigger O(1).
 * compile such automata would take considerably more time than DFA.
 */

/* === action algebra === */
struct Omega {};
template<size_t I> struct Set { static constexpr size_t i = I; };
template<typename... As> struct Seq {};

template<typename A1, typename A2> struct CatAction;
template<> struct CatAction<Omega, Omega> { using type = Omega; };
template<size_t I> struct CatAction<Omega, Set<I>> { using type = Set<I>; };
template<typename... As> struct CatAction<Omega, Seq<As...>> { using type = Seq<As...>; };
template<size_t I> struct CatAction<Set<I>, Omega> { using type = Set<I>; };
template<size_t I1, size_t I2> struct CatAction<Set<I1>, Set<I2>> { using type = Seq<Set<I1>, Set<I2>>; };
template<size_t I, typename... As> struct CatAction<Set<I>, Seq<As...>> { using type = Seq<Set<I>, As...>; };
template<typename... As> struct CatAction<Seq<As...>, Omega> { using type = Seq<As...>; };
template<typename... As, size_t I> struct CatAction<Seq<As...>, Set<I>> { using type = Seq<As..., Set<I>>; };
template<typename... As1, typename... As2> struct CatAction<Seq<As1...>, Seq<As2...>> { using type = Seq<As1..., As2...>; };
template<typename Seq, typename A> using CarAction_t = CatAction<Seq, A>::type;


namespace tnfa {

/* === v notation === */
template<typename List, typename ListOrAction, typename Acc> struct Product;
template<typename Head, typename... Tails, typename A, typename Acc> struct Product<TypeList<Head, Tails...>, A, Acc> {
  using TmpAcc = PushBackUnique<Acc, typename CatAction<Head, A>::type>::type;
  using type = Product<TypeList<Tails...>, A, TmpAcc>::type;
};
template<typename A, typename Acc> struct Product<TypeList<>, A, Acc> {
  using type = Acc;
};
template<typename List1, typename Head, typename... Tails, typename Acc> struct Product<List1, TypeList<Head, Tails...>, Acc> {
  using TmpAcc = Product<List1, Head, Acc>::type;
  using type = Product<List1, TypeList<Tails...>, TmpAcc>::type;
};
template<typename List1, typename Acc> struct Product<List1, TypeList<>, Acc> {
  using type = Acc;
};
template<typename Acc> struct Product<TypeList<>, TypeList<>, Acc> {
  using type = Acc;
};

template<typename RE> struct v;
template<> struct v<EmptySet> { using type = TypeList<>; };
template<> struct v<Epsilon> { using type = TypeList<Omega>; };
template<char C> struct v<Char<C>> { using type = TypeList<>; };
template<size_t I> struct v<SetSlot<I>> { using type = TypeList<Set<I>>; };
template<typename R, typename S> struct v<Or<R, S>> { using type = JoinUnique<v<R>, v<S>>::type; };
template<typename R, typename S> struct v<Concat<R, S>> { using type = Product<v<R>, v<S>, TypeList<>>::type; };
template<typename R> struct v<Closure<R>> { using type = TypeList<Omega>; };


/* === extended brzozowski derivative === */
// TODO


/* === TDFA builder === */
// TODO


/* === table builder, convert sparse graph representation into jump table, action list and other auxiliary structure === */
// TODO

} /* namespace tnfa */

} /* namespace impl */


/* === interface === */
template<impl::FixedString Pattern>
class Match {
public:
  static bool eval(std::string_view str) {
    std::size_t state = 0;
    for (const char& ch : str) {
      if (!impl::is_visible_char(ch)) [[unlikely]] return false;
      int32_t nxt = dfa_trans_table[state][static_cast<std::size_t>(ch)];
      if (nxt < 0) return false;
      state = static_cast<std::size_t>(nxt);
    }
    return dfa_is_accept_states[state];
  }

private:
  using Re = typename impl::RegexScan<Pattern>::type;
  using NoActionRe = typename impl::dfa::RemoveAllAction<Re>::type;
  using DFAStatesList = impl::dfa::AllStatesList<NoActionRe>;
  using DFAEdgesList  = impl::dfa::AllEdgesList<NoActionRe>;
  static constexpr std::size_t nr_dfa_states = DFAStatesList::length;
  static constexpr auto dfa_trans_table = impl::dfa::BuildTable<DFAEdgesList>::template make<nr_dfa_states>();
  static constexpr auto dfa_is_accept_states = impl::dfa::BuildAccepts<DFAStatesList>::make();
};

template<impl::FixedString Pattern>
class Replace {
public:
  static std::string eval(std::string_view replace_rule, std::string_view str) {
    // TODO: implement me with TNFA
    throw std::runtime_error("not implemented");
  }
};

} /* namespace onre */

#endif /* !ONRE_REGEX_HPP_ */
