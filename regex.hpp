#ifndef ONRE_REGEX_HPP
#define ONRE_REGEX_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <sstream>
#include <tuple>

namespace onre {
namespace impl {

/* === type list, a linear container of types === */
template<typename... Ts> struct TypeList {
  template<typename T>
  static constexpr bool Contains = (std::is_same_v<T, Ts> || ...);

  template<size_t Idx>
  using At = std::tuple_element_t<Idx, std::tuple<Ts...>>;

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

  static constexpr size_t length = sizeof...(Ts);

  template<template<typename, typename> typename IsLess>
  static constexpr bool IsInOrder = [] {
    if constexpr (length < 2) {
      return true;
    } else {
      using Tuple = std::tuple<Ts...>;
      return []<std::size_t... Is>(std::index_sequence<Is...>) {
        return ((!IsLess<std::tuple_element_t<Is + 1, Tuple>,
                      std::tuple_element_t<Is, Tuple>>::value) && ...);
      }(std::make_index_sequence<length - 1>{});
    }
  }();
};

template<typename T> struct IsTypeList : std::false_type {};
template<typename... Es> struct IsTypeList<TypeList<Es...>> : std::true_type {};

template<typename List1, typename List2> struct CatList;
template<typename... Ts1, typename... Ts2>
struct CatList<TypeList<Ts1...>, TypeList<Ts2...>> {
  using type = TypeList<Ts1..., Ts2...>;
};

template<typename List, typename T> struct PushFront;
template<typename... Ts, typename T>
struct PushFront<TypeList<Ts...>, T> {
  using type = TypeList<T, Ts...>;
};

template<typename List, typename T> struct PushBack;
template<typename... Ts, typename T>
struct PushBack<TypeList<Ts...>, T> {
  using type = TypeList<Ts..., T>;
};

template<typename List> struct PopFront { using type = List; };
template<typename Head, typename... Tails>
struct PopFront<TypeList<Head, Tails...>> {
  using type = TypeList<Tails...>;
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
    FilterImpl<IsKeep, TypeList<Tails...>, typename PushBack<Acc, Head>::type>,
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

template<template<typename, typename> typename IsLess, typename List, typename T>
struct InsertElem;
template<template<typename, typename> typename IsLess, typename T>
struct InsertElem<IsLess, TypeList<>, T> {
  using type = TypeList<T>;
};
template<template<typename, typename> typename IsLess, typename Head, typename... Tails, typename T>
struct InsertElem<IsLess, TypeList<Head, Tails...>, T> {
  using type = std::conditional_t<
    IsLess<T, Head>::value,
    std::type_identity<TypeList<T, Head, Tails...>>,
    PushFront<typename InsertElem<IsLess, TypeList<Tails...>, T>::type, Head>
  >::type;
};
template<template<typename, typename> typename IsLess, typename List> struct InsertSort {
  template<typename Acc, typename Head>
  struct Merge { using type = typename InsertElem<IsLess, Acc, Head>::type; };
  using type = LeftFold<Merge, List, TypeList<>>::type;
};

template<template<typename, typename> typename IsLess, typename List>
struct Sort {
  using type = std::conditional_t<
    List::template IsInOrder<IsLess>,
    std::type_identity<List>,
    InsertSort<IsLess, List>
  >::type;
};


/* === fixed string, a string container enabling compile-time visiting === */
template<size_t N>
struct FixedString {
  char data[N]; /* include '\0' */

  constexpr FixedString(const char (&str)[N]) {
    std::copy_n(str, N, data);
  }

  constexpr FixedString(const FixedString&) noexcept = default;
  constexpr FixedString(FixedString&&) noexcept = default;
  constexpr FixedString& operator=(const FixedString&) noexcept = default;
  constexpr FixedString& operator=(FixedString&&) noexcept = default;

  static constexpr size_t length = N - 1;
  constexpr const char* c_str() const { return data; }
  constexpr char operator[](size_t i) const { return data[i]; }
};
template<size_t N>
FixedString(const char (&str)[N]) -> FixedString<N>;

template<size_t Length>
struct FixedStringView {
  const char* data;

  constexpr FixedStringView(const char* str) : data(str) {}

  constexpr FixedStringView(const FixedStringView&) noexcept = default;
  constexpr FixedStringView(FixedStringView&&) noexcept = default;
  constexpr FixedStringView& operator=(const FixedStringView&) noexcept = default;
  constexpr FixedStringView& operator=(FixedStringView&&) noexcept = default;

  static constexpr size_t length = Length;
  constexpr const char* c_str() const { return data; }
  constexpr char operator[](size_t i) const { return data[i]; }
};

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

/* e* <=> e */
template<> struct Simplify<Closure<EmptySet>> { using type = Epsilon; };
/* 0* <=> 0 */
template<> struct Simplify<Closure<Epsilon>> { using type = EmptySet; };

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

/* recursive */
template<typename Expr, typename Simplified, bool IsSame>
struct SimplifyFixedPoint;
template<typename Expr, typename Simplified>
struct SimplifyFixedPoint<Expr, Simplified, true> {
  using type = Simplified;
};
template<typename Expr, typename Simplified>
struct SimplifyFixedPoint<Expr, Simplified, false> {
  using type = typename Simplify<Simplified>::type;
};
template<typename L, typename R>
struct Simplify<Or<L, R>> {
  using simplified = Or<typename Simplify<L>::type, typename Simplify<R>::type>;
  static constexpr bool is_same = std::is_same<simplified, Or<L, R>>::value;
  using type = typename SimplifyFixedPoint<Or<L, R>, simplified, is_same>::type;
};
template<typename L, typename R>
struct Simplify<Concat<L, R>> {
  using simplified = Concat<typename Simplify<L>::type, typename Simplify<R>::type>;
  static constexpr bool is_same = std::is_same<simplified, Concat<L, R>>::value;
  using type = typename SimplifyFixedPoint<Concat<L, R>, simplified, is_same>::type;
};
template<typename R>
struct Simplify<Closure<R>> {
  using simplified = Closure<typename Simplify<R>::type>;
  static constexpr bool is_same = std::is_same<simplified, Closure<R>>::value;
  using type = typename SimplifyFixedPoint<Closure<R>, simplified, is_same>::type;
};


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
                | Atom '{' Number ',' '}' | Atom '{' Number ',' Number '}'
                | Atom '{' ',' Number '}'
    Atom        := '(' Regex ')' | CharGroup | CHAR | '.'
    CharGroup   := '[' CharSet ']' | '[' '^' CharSet ']'
    CharSet     := CharSetAtom CharSet | CharSetAtom
    CharSetAtom := [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | Escape
    CHAR        := [VALID CHAR] | Escape
    Escape      := '\' [VISIBLE CHAR] | '\' 'x' Number
    Empty input -> Epsilon
*/

/* forward declarations */
template<FixedStringView Pattern, size_t Pos, size_t CapIdx> struct ParseRegex;
template<FixedStringView Pattern, size_t Pos, size_t CapIdx> struct ParseTerm;
template<FixedStringView Pattern, size_t Pos, size_t CapIdx> struct ParseFactor;
template<FixedStringView Pattern, size_t Pos, size_t CapIdx> struct ParseAtom;
template<FixedStringView Pattern, size_t Pos> struct ParseCharGroup;
template<FixedStringView Pattern, size_t Pos> struct ParseCharSet;
template<FixedStringView Pattern, size_t Pos> struct ParseCharSetAtom;
template<FixedStringView Pattern, size_t Pos> struct ParseCHAR;
template<FixedStringView Pattern, size_t Pos> struct ParseEscape;

template<typename CharList>
struct CharListToOrSequential {
  template<typename X, typename Y> struct BuildOr { using type = Or<X, Y>; };
  using type = typename RightFold<BuildOr, CharList, EmptySet>::type;
};

template<typename CharList, typename Alphabet>
struct CharListNegation {
  template <typename Char>
  struct NotInList {
    static constexpr bool value = !CharList::template Contains<Char>;
  };
  using type = Filter<NotInList, Alphabet>::type;
};

template<FixedStringView Pattern, size_t Pos, int64_t Acc = 0> struct ParseDecimal {
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

template<FixedStringView Pattern, size_t Pos, size_t N, int64_t Acc = 0> struct ParseHexN {
  struct is_digit_impl {
    static constexpr char ch = Pattern[Pos];
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

template<char C, FixedStringView Pattern, size_t Pos>
struct EscapeImpl {
  using type = Char<Pattern[Pos + 1]>;
  static constexpr size_t next = Pos + 2;
};
using WordList = JoinUnique<
  typename BuildCharList<'a', 'z'>::type,
  typename JoinUnique<
    typename BuildCharList<'A', 'Z'>::type,
    typename PushBack<typename BuildCharList<'0', '9'>::type, Char<'_'>>::type
  >::type
>::type;
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'w', Pattern, Pos> {
  using type = CharListToOrSequential<WordList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'W', Pattern, Pos> {
  using type = CharListToOrSequential<typename CharListNegation<WordList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
using DigitList = BuildCharList<'0', '9'>::type;
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'d', Pattern, Pos> {
  using type = CharListToOrSequential<DigitList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'D', Pattern, Pos> {
  using type = CharListToOrSequential<typename CharListNegation<DigitList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
using WhitespaceList = TypeList<Char<' '>, Char<'\t'>, Char<'\x0B'>, Char<'\n'>, Char<'\f'>, Char<'\r'>>;
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'s', Pattern, Pos> {
  using type = CharListToOrSequential<WhitespaceList>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'S', Pattern, Pos> {
  using type = CharListToOrSequential<typename CharListNegation<WhitespaceList, Alphabet>::type>::type;
  static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'n', Pattern, Pos> {
  using type = Char<'\n'>; static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'t', Pattern, Pos> {
  using type = Char<'\t'>; static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'f', Pattern, Pos> {
  using type = Char<'\f'>; static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'r', Pattern, Pos> {
  using type = Char<'\r'>; static constexpr size_t next = Pos + 2;
};
template<FixedStringView Pattern, size_t Pos> struct EscapeImpl<'x', Pattern, Pos> {
  static_assert(
    Pos + 2 < Pattern.length && (
      (Pattern[Pos + 2] >= '0' && Pattern[Pos + 2] <= '9')
      || (Pattern[Pos + 2] >= 'a' && Pattern[Pos + 2] <= 'f')
      || (Pattern[Pos + 2] >= 'A' && Pattern[Pos + 2] <= 'F')
    ),
    "no value specified for `\\x`"
  );
  using HexParse = ParseHexN<Pattern, Pos + 2, 2>;
  using type = Char<HexParse::value>;
  static constexpr size_t next = HexParse::next;
};

template <FixedStringView Pattern, size_t Pos>
struct ParseEscape {
  static_assert(Pos + 1 < Pattern.length, "ParseEscape: cannot find escape character");
  static_assert(is_visible_char(Pattern[Pos + 1]), "ParseEscape: unknown character");
  using chosen = EscapeImpl<Pattern[Pos + 1], Pattern, Pos>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCHAR : [VALID CHAR] | Escape */
template <FixedStringView Pattern, size_t Pos>
struct ParseCHAR {
  static_assert(
    Pos < Pattern.length && (Pattern[Pos] == '\\' || is_valid_char(Pattern[Pos])),
    "ParseCHAR: unknown character"
  );

  struct impl_escape {
    using EscapeParse = ParseEscape<Pattern, Pos>;
    using type = EscapeParse::type;
    static constexpr size_t next = ParseEscape<Pattern, Pos>::next;
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

/* ParseCharSetAtom: [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | Escape */
template<FixedStringView Pattern, size_t Pos>
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
    using EscapeParse = ParseEscape<Pattern, Pos>;
    using type = TypeList<typename EscapeParse::type>;
    static constexpr size_t next = ParseEscape<Pattern, Pos>::next;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  static constexpr bool has_hyphen = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '-';
  using chosen = std::conditional_t<is_escape, impl_escape, std::conditional_t<has_hyphen, impl_seq, impl_char>>;
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharSet: CharSetAtom CharSet | CharSetAtom */
template<FixedStringView Pattern, size_t Pos>
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
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

/* ParseCharGroup: '[' CharSet ']' | '[' '^' CharSet ']' */
template<FixedStringView Pattern, size_t Pos>
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
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
};

template<typename Alphabet> struct FullMatch {
  template<typename X, typename Y> struct BuildOr { using type = Or<X, Y>; };
  using type = typename RightFold<BuildOr, Alphabet, EmptySet>::type;
};

/* ParseAtom: '(' Regex ')' | '[' CharSet ']' |  CHAR | '.' */
template<FixedStringView Pattern, size_t Pos, size_t CapIdx>
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
  using type = typename chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = chosen::next_cap_idx;
};

template<typename AtomType, int64_t Min, int64_t Max> struct BuildQuantifier {
  using type = Concat<AtomType, typename BuildQuantifier<AtomType, Min - 1, Max - 1>::type>;
};
template<typename AtomType, int64_t Max> struct BuildQuantifier<AtomType, 0, Max> {
  struct inf {
    using type = Closure<AtomType>;
  };
  struct non_inf {
    using type = Or<Epsilon, Concat<AtomType, typename BuildQuantifier<AtomType, 0, Max - 1>::type>>;
  };
  using type = std::conditional_t<Max < 0, inf, non_inf>::type;
};
template<typename AtomType> struct BuildQuantifier<AtomType, 0, 0> {
  using type = Epsilon;
};

/* ParseFactor: Atom ('*')? | Atom ('+')? | Atom ('?')?
 *            | Atom '{' Number ',' '}' | Atom '{' Number ',' Number '}'
 *            | Atom '{' ',' Number '}' | '{' Number '}'
 */
template<FixedStringView Pattern, size_t Pos, size_t CapIdx>
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
    using ParseMin = ParseDecimal<Pattern, Atom::next + 1>;
    static constexpr int64_t Min = Pattern[Atom::next + 1] == ',' ? 0 : ParseMin::value;
    static_assert(ParseMin::next < Pattern.length && (Pattern[ParseMin::next] == ',' || Pattern[ParseMin::next] == '}'),
      "ParseFactor: incomplete quantifier");
    struct single_num {
      static constexpr int64_t Max = Min;
      static constexpr size_t next = ParseMin::next + 1;
    };
    struct multiple_num {
      using ParseMax = ParseDecimal<Pattern, ParseMin::next + 1>;
      static constexpr int64_t Max = Pattern[ParseMin::next + 1] == '}' ? -1 : ParseMax::value;
      static constexpr size_t next = ParseMax::next + 1;
    };
    using chosen = std::conditional_t<Pattern[ParseMin::next] == '}', single_num, multiple_num>;
    static constexpr int64_t Max = chosen::Max;
    static_assert(Max < 0 || Min <= Max, "ParseFactor: invalid quantifier");
    using type = BuildQuantifier<typename Atom::type, Min, Max>::type;
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
  using type = chosen::type;
  static constexpr size_t next = chosen::next;
  static constexpr size_t next_cap_idx = Atom::next_cap_idx;
};

/* ParseTerm := Factor Term | (empty -> Epsilon) */
template<FixedStringView Pattern, size_t Pos, size_t CapIdx>
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
template<FixedStringView Pattern, size_t Pos, size_t CapIdx>
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

template<FixedStringView Pattern>
struct RegexScan {
  using Parse = ParseRegex<Pattern, 0, 1>;
  using type = Simplify<Concat<SetSlot<0>, Concat<typename Parse::type, SetSlot<1>>>>::type;
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
struct Edge {
  static constexpr std::size_t from = From;
  static constexpr char ch = C;
  static constexpr std::size_t to = To;
};

template<char C, typename State>
struct CharStatePair {
  static constexpr char c = C;
  using state = State;
};

template<typename CharStatePairAcc, typename State, typename Alphabet> struct DerivNewStates;
template<typename Acc, typename S> struct DerivNewStates<Acc, S, TypeList<>> {
  using type = Acc;
};
template<typename Acc, typename S, char C, typename... Tails>
struct DerivNewStates<Acc, S, TypeList<Char<C>, Tails...>> {
  using Der = typename Derivative<typename S::re, C>::type;
  using type = std::conditional_t<
    std::is_same_v<Der, EmptySet>,
    std::type_identity<Acc>,
    DerivNewStates<
      typename PushBack<Acc, CharStatePair<C, State<Der>>>::type,
      S,
      TypeList<Tails...>
    >
  >::type;
};

template<typename StateAcc, typename EdgeAcc, typename ToBeProcessList, typename StartState, typename NewCharStates>
struct PushNewStates;
template<typename SA, typename EA, typename TBP, typename StartState>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<>> {
  using StateAcc = SA;
  using EdgeAcc = EA;
  using ToBeProcessList = TBP;
};
template<typename SA, typename EA, typename TBP, typename StartState, typename HeadPair, typename... TailPairs>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<HeadPair, TailPairs...>> {
  using FromState = StartState;
  using ToState = typename HeadPair::state;
  static constexpr char C = HeadPair::c;
  static constexpr bool IsStateNew = !SA::template Contains<ToState>;
  using NextStateAcc = PushBackUnique<SA, ToState>::type;
  using NextToBeProcessList = std::conditional_t<IsStateNew, PushBack<TBP, ToState>, std::type_identity<TBP>>::type;
  using NextEdgeAcc = typename PushBack<
    EA,
    Edge<
      NextStateAcc::template IndexOf<FromState>,
      C,
      NextStateAcc::template IndexOf<ToState>
    >
  >::type;
  using NextIt = PushNewStates<NextStateAcc, NextEdgeAcc, NextToBeProcessList, StartState, TypeList<TailPairs...>>;
  using StateAcc = typename NextIt::StateAcc;
  using EdgeAcc = typename NextIt::EdgeAcc;
  using ToBeProcessList = typename NextIt::ToBeProcessList;
};

template<typename StateAcc, typename EdgeAcc, typename ToBeProcessList> struct BuildDFA;
template<typename StateAcc, typename EdgeAcc> struct BuildDFA<StateAcc, EdgeAcc, TypeList<>> {
  using States = StateAcc;
  using Edges = EdgeAcc;
};
template<typename StateAcc, typename EdgeAcc, typename StateHead, typename... StateTails>
struct BuildDFA<StateAcc, EdgeAcc, TypeList<StateHead, StateTails...>> {
  using NewCharStates = DerivNewStates<TypeList<>, StateHead, typename First<typename StateHead::re, TypeList<>>::type>::type;
  using Processed = PushNewStates<StateAcc, EdgeAcc, TypeList<StateTails...>, StateHead, NewCharStates>;
  using NextIt = BuildDFA<typename Processed::StateAcc, typename Processed::EdgeAcc, typename Processed::ToBeProcessList>;
  using States = NextIt::States;
  using Edges = NextIt::Edges;
};

template<typename RE>
struct AllStatesAndEdgesGenerator {
public:
  using type = BuildDFA<TypeList<State<RE>>, TypeList<>, TypeList<State<RE>>>;
  using States = typename type::States;
  using Edges  = typename type::Edges;
};

/* aliases */
template<typename RE> using AllStateEdgePair = typename AllStatesAndEdgesGenerator<RE>::type;
template<typename RE> using AllStatesList = typename AllStatesAndEdgesGenerator<RE>::States;
template<typename RE> using AllEdgesList  = typename AllStatesAndEdgesGenerator<RE>::Edges;


/* === table builder, convert sparse graph representation into jump table representation === */
template<size_t NrStates, typename EdgesList>
struct BuildTable;
template<size_t NrStates, typename... Edges>
struct BuildTable<NrStates, TypeList<Edges...>> {
  static constexpr std::array<std::array<int32_t, nr_ascii_char>, NrStates> make() {
    std::array<std::array<int32_t, nr_ascii_char>, NrStates> table{};
    for (auto &row : table) row.fill(-1);
    ((table[Edges::from][static_cast<std::size_t>(Edges::ch)] = Edges::to), ...);
    return table;
  }
};

template<typename StatesList>
struct BuildAccepts;
template<typename... Ss>
struct BuildAccepts<TypeList<Ss...>> {
  static constexpr std::array<bool, sizeof...(Ss)> make() {
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
struct Omega { static constexpr size_t length = 0;};
template<size_t I> struct Set { static constexpr size_t i = I, length = 1; };
template<typename... As> struct Seq { static constexpr size_t length = (As::length + ...); };

template<typename A> struct is_omega : std::false_type {};
template<> struct is_omega<Omega> : std::true_type {};
template<typename A> struct is_set : std::false_type {};
template<size_t I> struct is_set<Set<I>> : std::true_type {};
template<typename A> struct is_seq : std::false_type {};
template<typename... As> struct is_seq<Seq<As...>> : std::true_type {};

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
template<typename List, typename Action> struct ProductAction {
  template <typename A> struct AddAction { using type = CatAction<A, Action>::type; };
  using type = Map<AddAction, List>::type;
};
template<typename List1, typename List2, typename Acc> struct Product;
template<typename List1, typename Head, typename... Tails, typename Acc> struct Product<List1, TypeList<Head, Tails...>, Acc> {
  using TmpAcc = ProductAction<List1, Head>::type;
  using type = Product<List1, TypeList<Tails...>, TmpAcc>::type;
};
template<typename List1, typename Acc> struct Product<List1, TypeList<>, Acc> {
  using type = Acc;
};

template<typename RE> struct v;
template<> struct v<EmptySet> { using type = TypeList<>; };
template<> struct v<Epsilon> { using type = TypeList<Omega>; };
template<char C> struct v<Char<C>> { using type = TypeList<>; };
template<size_t I> struct v<SetSlot<I>> { using type = TypeList<Set<I>>; };
template<typename R, typename S> struct v<Or<R, S>> {
  using type = JoinUnique<typename v<R>::type, typename v<S>::type>::type;
};
template<typename R, typename S> struct v<Concat<R, S>> {
  using type = Product<typename v<R>::type, typename v<S>::type, TypeList<>>::type;
};
template<typename R> struct v<Closure<R>> { using type = TypeList<Omega>; };


/* === extended brzozowski derivative === */
template <typename Remain, typename Action> struct DerivedPair {
  using remain = Remain; using action = Action;
};

template <typename RE, char C> struct Derivative;
/* d0/dx de/dx = d<i>/dx = 0 */
template <char C> struct Derivative<EmptySet, C> { using type = TypeList<>; };
template <char C>
struct Derivative<Epsilon, C> { using type = TypeList<>; };
template <size_t I, char C> struct Derivative<SetSlot<I>, C> { using type = TypeList<>; };
/* dy/dx = x == y ? {(e, o)} : 0 */
template <char y, char C>
struct Derivative<Char<y>, C> {
  using type = std::conditional_t<
    y == C,
    TypeList<DerivedPair<Epsilon, Omega>>,
    TypeList<>
  >;
};
/* d(R|S)/dx = dR/dx U dS/dx */
template <typename R, typename S, char C>
struct Derivative<Or<R, S>, C> {
  using dr = Derivative<R, C>::type;
  using ds = Derivative<S, C>::type;
  using type = JoinUnique<dr, ds>::type;
};
/* d(RS)/dx = {(R'S, a):(R',a) in dR/dx} U {(S', ab):a in v(R), (S', b) in dS/dx} */
template <typename R, typename S, char C>
struct Derivative<Concat<R, S>, C> {
  template <typename Pair>
  struct MapFunc1 {
    using type = DerivedPair<
      typename Simplify<Concat<typename Pair::remain, S>>::type,
      typename Pair::action
    >;
  };
  using Part1 = Map<MapFunc1, typename Derivative<R, C>::type>::type;

  template <typename Acc, typename vRList, typename SDList> struct Part2Generator;
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
    using type = Part2Generator<
      typename JoinUnique<Acc, typename Map<MapFunc2, SDList>::type>::type,
      TypeList<vRTails...>,
      SDList
    >::type;
  };
  using Part2 = Part2Generator<TypeList<>, typename v<R>::type, typename Derivative<S, C>::type>::type;

  using type = JoinUnique<Part1, Part2>::type;
};
/* d(R*)/dx = {(R'R*,a):(R',a) in dR/dx} */
template <typename R, char C>
struct Derivative<Closure<R>, C> {
  template <typename Pair>
  struct MapFunc {
    using type = DerivedPair<
      typename Simplify<Concat<typename Pair::remain, Closure<R>>>::type,
      typename Pair::action
    >;
  };

  using type = Map<MapFunc, typename Derivative<R, C>::type>::type;
};


/* === TNFA builder === */
template<typename R>
struct State {
  using re = R;
  static constexpr bool accepting = Nullable<R>::value;
  using AcceptActions = v<R>::type;
};

template<size_t From, char C, typename Action, size_t To>
struct Edge {
  static constexpr std::size_t from = From;
  static constexpr char ch = C;
  using action = Action;
  static constexpr size_t to = To;
};

template<char C, typename State, typename Action>
struct CharStateAction {
  static constexpr char c = C;
  using state = State;
  using action = Action;
};

template<typename CharStateActionAcc, typename State, typename Alphabet> struct DerivNewStates;
template<typename Acc, typename S> struct DerivNewStates<Acc, S, TypeList<>> {
  using type = Acc;
};

template<typename Acc, typename S, char C, typename... Tails>
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
  using type = std::conditional_t<
    std::is_same_v<Der, TypeList<>>,
    std::type_identity<Acc>,
    DerivNewStates<
      typename CatList<Acc, typename Map<AddChar, Der>::type>::type,
      S,
      TypeList<Tails...>
    >
  >::type;
};

template<typename StateAcc, typename EdgeAcc, typename ToBeProcessList, typename StartState, typename NewCharStates>
struct PushNewStates;
template<typename SA, typename EA, typename TBP, typename StartState>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<>> {
  using StateAcc = SA;
  using EdgeAcc = EA;
  using ToBeProcessList = TBP;
};
template<typename SA, typename EA, typename TBP, typename StartState, typename HeadTuple, typename... TailPairs>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<HeadTuple, TailPairs...>> {
  using FromState = StartState;
  using ToState = typename HeadTuple::state;
  static constexpr char C = HeadTuple::c;
  using Action = HeadTuple::action;
  static constexpr bool IsStateNew = !SA::template Contains<ToState>;
  using NextStateAcc = PushBackUnique<SA, ToState>::type;
  using NextToBeProcessList = std::conditional_t<IsStateNew, PushBack<TBP, ToState>, std::type_identity<TBP>>::type;
  using NextEdgeAcc = typename PushBack<
    EA,
    Edge<
      NextStateAcc::template IndexOf<FromState>,
      C,
      Action,
      NextStateAcc::template IndexOf<ToState>
    >
  >::type;
  using NextIt = PushNewStates<NextStateAcc, NextEdgeAcc, NextToBeProcessList, StartState, TypeList<TailPairs...>>;
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
  using NewCharStates = DerivNewStates<TypeList<>, StateHead, typename First<typename StateHead::re, TypeList<>>::type>::type;
  using Processed = PushNewStates<StateAcc, EdgeAcc, TypeList<StateTails...>, StateHead, NewCharStates>;
  using NextIt = BuildTNFA<typename Processed::StateAcc, typename Processed::EdgeAcc, typename Processed::ToBeProcessList>;
  using States = NextIt::States;
  using Edges = NextIt::Edges;
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

/* === table builder, convert sparse graph representation into jump table, action list and other auxiliary structure === */
template<typename RE> struct NrUsedSlots {
  static constexpr size_t value = 0;
};
template<size_t I> struct NrUsedSlots<SetSlot<I>> {
  static constexpr size_t value = I + 1;
};
template<typename R, typename S> struct NrUsedSlots<Or<R, S>> {
  static constexpr size_t value = std::max(NrUsedSlots<R>::value, NrUsedSlots<S>::value);
};
template<typename R, typename S> struct NrUsedSlots<Concat<R, S>> {
  static constexpr size_t value = std::max(NrUsedSlots<R>::value, NrUsedSlots<S>::value);
};
template<typename R> struct NrUsedSlots<Closure<R>> {
  static constexpr size_t value = NrUsedSlots<R>::value;
};

template<typename Edges> struct MaxTransActionLength;
template<> struct MaxTransActionLength<TypeList<>> { static constexpr size_t value = 0; };
template<typename... Edges> struct MaxTransActionLength<TypeList<Edges...>> {
  static constexpr size_t value = std::max({ Edges::action::length... });
};

template<typename List> struct MaxActionLengthInList;
template<> struct MaxActionLengthInList<TypeList<>> { static constexpr size_t value = 0; };
template<typename... Actions> struct MaxActionLengthInList<TypeList<Actions...>> {
  static constexpr size_t value = std::max({ Actions::length... });
};

template<typename States> struct MaxAcceptActionLength;
template<> struct MaxAcceptActionLength<TypeList<>> { static constexpr size_t value = 0; };
template<typename... States> struct MaxAcceptActionLength<TypeList<States...>> {
  static constexpr size_t value = std::max({ MaxActionLengthInList<typename States::AcceptActions>::value... });
};

template<size_t NrStates, typename EdgeList> struct BuildTransTable;
template<size_t NrStates, typename... Edges> struct BuildTransTable<NrStates, TypeList<Edges...>> {
  static constexpr std::array<std::array<std::array<bool, NrStates>, nr_ascii_char>, NrStates> make() {
    std::array<std::array<std::array<bool, NrStates>, nr_ascii_char>, NrStates> result{};
    for (auto& state_table : result) for (auto& char_table : state_table) char_table.fill(false);
    ((result[Edges::from][static_cast<size_t>(Edges::ch)][Edges::to] = true), ...);
    return result;
  }
};

/* remain the edge with shortest action among edges having same From, Char and To */
template<typename EdgeList> struct RemoveConflictEdge {
  template<typename Edge, typename ProcessingEdgeList> struct IsEdgeNeedRetainImpl;
  template<typename Edge> struct IsEdgeNeedRetainImpl<Edge, TypeList<>> {
    static constexpr bool value = true;
  };
  template<typename Edge, typename Head, typename... Tails>
  struct IsEdgeNeedRetainImpl<Edge, TypeList<Head, Tails...>> {
    static constexpr bool value = IsEdgeNeedRetainImpl<Edge, TypeList<Tails...>>::value;
  };
  template<size_t F, char C, size_t T, typename A1, typename A2, typename... Tails>
  struct IsEdgeNeedRetainImpl<Edge<F, C, A1, T>, TypeList<Edge<F, C, A2, T>, Tails...>> {
    static constexpr bool value = A1::length <= A2::length && IsEdgeNeedRetainImpl<Edge<F, C, A1, T>, TypeList<Tails...>>::value;
  };

  template<typename Edge> struct IsEdgeNeedRetain {
    static constexpr bool value = IsEdgeNeedRetainImpl<Edge, EdgeList>::value;
  };

  using type = Filter<IsEdgeNeedRetain, EdgeList>::type;
};

template<typename StateList> struct BuildAcceptTable;
template<typename... States> struct BuildAcceptTable<TypeList<States...>> {
  static constexpr std::array<bool, sizeof...(States)> make() {
    return std::array<bool, sizeof...(States)>{ States::accepting... };
  }
};

template<size_t NrStates, size_t MaxTransActionLength, typename EdgeList> struct BuildTransActionTable;
template<size_t NrStates, size_t MaxTransActionLength, typename... Edges>
struct BuildTransActionTable<NrStates, MaxTransActionLength, TypeList<Edges...>> {
  static constexpr std::array<std::array<std::array<std::array<int32_t, MaxTransActionLength>, NrStates>, nr_ascii_char>, NrStates> make() {
    std::array<std::array<std::array<std::array<int32_t, MaxTransActionLength>, NrStates>, nr_ascii_char>, NrStates> result{};
    for (auto& from_state_table : result)
      for (auto& char_table : from_state_table)
        for (auto& action_list : char_table)
          action_list.fill(-1);

    (([&]<typename Edge>(Edge) {
      using Action = typename Edge::action;
      auto& action_list = result[Edge::from][Edge::ch][Edge::to];
      action_list.fill(-1);
      if constexpr (is_omega<Action>::value) {
        /* ignore */
      } else if constexpr (is_set<Action>::value) {
        static_assert(MaxTransActionLength > 0, "bad max trans action length");
        action_list[0] = static_cast<int32_t>(Action::i);
      } else if constexpr (is_seq<Action>::value) {
        [&]<typename... As>(Seq<As...>) {
          static_assert(MaxTransActionLength >= sizeof...(As), "bad max trans action length");
          size_t idx = 0;
          ((action_list[idx++] = static_cast<int32_t>(As::i)), ...);
        }(Action{});
      } else {
        static_assert(!std::is_same<Edges, Edges>::value, "unknown action type");
      }
    }(Edges{})), ...);

    return result;
  }
};

template<size_t MaxAcceptActionLength, typename StateList> struct BuildAcceptActionTable;
template<size_t MaxAcceptActionLength, typename... States>
struct BuildAcceptActionTable<MaxAcceptActionLength, TypeList<States...>> {
  static constexpr std::array<std::array<int32_t, MaxAcceptActionLength>, sizeof...(States)> make() {
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

  template<typename ActionList, typename CurLongestAction, size_t CurShortestLen> struct LongestAction;
  template<typename CurLongestAction, size_t CurShortestLen> struct LongestAction<TypeList<>, CurLongestAction, CurShortestLen> {
    using type = CurLongestAction;
  };
  template<typename HeadAction, typename... TailActions, typename CurLongestAction, size_t CurShortestLen>
  struct LongestAction<TypeList<HeadAction, TailActions...>, CurLongestAction, CurShortestLen> {
    using type = std::conditional_t<
      (HeadAction::length > CurShortestLen),
      LongestAction<TypeList<TailActions...>, HeadAction, HeadAction::length>,
      LongestAction<TypeList<TailActions...>, CurLongestAction, CurShortestLen>
    >::type;
  };
};

template<size_t Value> struct Num { static constexpr size_t value = Value; };
template<size_t X, size_t Y> struct NumPair { static constexpr size_t x = X, y = Y; };
template<typename Acc, FixedStringView Pattern, size_t Idx, size_t NrSeenGroup, typename OpeningGroupsIdx, typename ClosedGroups> struct MutualGroups {
  struct Impl {
    struct Open {
      static constexpr size_t OpenedIdx = NrSeenGroup;
      template<typename ClosedIdx> struct MapFunc { using type = NumPair<ClosedIdx::value, OpenedIdx>; };
      using NextAcc = JoinUnique<Acc, typename Map<MapFunc, ClosedGroups>::type>::type;
      using NextOpeningGroupsIdx = PushFront<OpeningGroupsIdx, Num<OpenedIdx>>::type;
      using type = MutualGroups<NextAcc, Pattern, Idx + 1, NrSeenGroup + 1, NextOpeningGroupsIdx, ClosedGroups>::type;
    };
    struct Close {
      static constexpr size_t ClosedIdx = OpeningGroupsIdx::template At<0>::value;
      using NextOpeningGroupsIdx = PopFront<OpeningGroupsIdx>::type;
      using NextClosedGroups = PushBack<ClosedGroups, Num<ClosedIdx>>::type;
      using type = MutualGroups<Acc, Pattern, Idx + 1, NrSeenGroup, NextOpeningGroupsIdx, NextClosedGroups>::type;
    };
    struct Other {
      using type = MutualGroups<Acc, Pattern, Idx + 1, NrSeenGroup, OpeningGroupsIdx, ClosedGroups>::type;
    };
    using type = std::conditional_t<Pattern[Idx] == '(', Open, std::conditional_t<Pattern[Idx] == ')', Close, Other>>::type;
  };
  using type = std::conditional_t<Idx >= Pattern.length, std::type_identity<Acc>, Impl>::type;
};

template<size_t NrGroups, typename MutualPairList> struct BuildMutualTable;
template<size_t NrGroups, typename... MutualPairs> struct BuildMutualTable<NrGroups, TypeList<MutualPairs...>> {
  static constexpr std::array<std::array<bool, NrGroups>, NrGroups> make() {
    std::array<std::array<bool, NrGroups>, NrGroups> result;
    for (auto& line : result) line.fill(false);
    ((result[MutualPairs::x][MutualPairs::y] = result[MutualPairs::y][MutualPairs::x] = true), ...);
    return result;
  }
};


} /* namespace tnfa */

} /* namespace impl */


/* === interface === */
class replacement_not_matched : public std::runtime_error {
public:
  replacement_not_matched(const char* msg) : std::runtime_error(msg) {}
  replacement_not_matched(const std::string& msg) : std::runtime_error(msg) {}
  replacement_not_matched(std::string&& msg) : std::runtime_error(msg) {}
};
class invalid_replacement_rule : public std::runtime_error {
public:
  invalid_replacement_rule(const char* msg) : std::runtime_error(msg) {}
  invalid_replacement_rule(const std::string& msg) : std::runtime_error(msg) {}
  invalid_replacement_rule(std::string&& msg) : std::runtime_error(msg) {}
};

template<impl::FixedString Pattern>
class Match {
public:
  static bool eval(std::string_view str) {
    std::size_t state = 0;
    for (const char& ch : str) {
      if (ch < 0 || ch > 128) [[unlikely]] return false;
      int32_t nxt = dfa_trans_table[state][static_cast<std::size_t>(ch)];
      if (nxt < 0) return false;
      state = static_cast<std::size_t>(nxt);
    }
    return dfa_is_accept_states[state];
  }

private:
  static constexpr auto pattern_view = impl::FixedStringView<Pattern.length>(Pattern.c_str());
  using Re = typename impl::RegexScan<pattern_view>::type;
  using NoActionRe = typename impl::dfa::RemoveAllAction<Re>::type;
  using DFAStatesList = impl::dfa::AllStatesList<NoActionRe>;
  using DFAEdgesList  = impl::dfa::AllEdgesList<NoActionRe>;
  static constexpr std::size_t nr_dfa_states = DFAStatesList::length;
  static constexpr auto dfa_trans_table = impl::dfa::BuildTable<nr_dfa_states, DFAEdgesList>::make();
  static constexpr auto dfa_is_accept_states = impl::dfa::BuildAccepts<DFAStatesList>::make();
};

template<impl::FixedString Pattern>
class Replace {
public:
  static std::string eval(std::string_view replace_rule, std::string_view str) {
    if (!Match<Pattern>::eval(str)) {
      throw replacement_not_matched("failed to match");
    }

    auto slot_file1 = new_slot_file(), slot_file2 = new_slot_file();
    std::array<bool, nr_states> active_states1 {}, active_states2 {};
    active_states1.fill(false); active_states2.fill(false);

    auto* cur_slot_file = &slot_file1, * next_slot_file = &slot_file2;
    auto* cur_active_state = &active_states1, * next_active_state = &active_states2;

    (*cur_active_state)[0] = true;
    for (size_t idx = 0; idx < str.size(); idx++) {
      char ch = str[idx];
      if (ch == '\0') break;
      next_active_state->fill(false);
      for (size_t state = 0; state < nr_states; state++) {
        if (!(*cur_active_state)[state]) continue;
        for (size_t next_state = 0; next_state < nr_states; next_state++) {
          if (!trans_table[state][static_cast<size_t>(ch)][next_state]) continue;
          auto next_slot_line = apply_action(
            (*cur_slot_file)[state], trans_action_table[state][static_cast<size_t>(ch)][next_state], idx
          );
          if (!(*next_active_state)[next_state] || need_change((*next_slot_file)[next_state], next_slot_line)) {
            (*next_slot_file)[next_state] = next_slot_line;
          }
          (*next_active_state)[next_state] = true;
        }
      }
      std::swap(cur_slot_file, next_slot_file);
      std::swap(cur_active_state, next_active_state);
    }

    bool is_final_line_updated = false;
    SlotLine final_line {};
    for (size_t state = 0; state < nr_states; state++) {
      if (!(*cur_active_state)[state]) continue;
      if (!accept_table[state]) continue;
      auto after_accept = apply_action((*cur_slot_file)[state], accept_action_table[state], str.size());
      if (!is_final_line_updated || need_change(final_line, after_accept)) {
        is_final_line_updated = true;
        final_line = after_accept;
      }
    }

    std::ostringstream result_buffer;

    for (size_t idx = 0; idx < replace_rule.size(); idx++) {
      char ch = replace_rule[idx];
      if (ch != '$') {
        result_buffer << ch;
        continue;
      }
      if (idx + 1 >= replace_rule.size()) {
        throw invalid_replacement_rule("missing argument for `$`");
      }
      idx++;
      if (replace_rule[idx] == '$') {
        result_buffer << '$';
        continue;
      } else if (is_digit(replace_rule[idx])) {
        size_t group_idx = replace_rule[idx] - '0';
        while (idx + 1 < replace_rule.size() && is_digit(replace_rule[idx + 1])) {
          idx++;
          group_idx = 10 * group_idx + replace_rule[idx] - '0';
        }
        if (group_idx >= nr_capture_group) {
          throw invalid_replacement_rule("undefined capture group `$" + std::to_string(group_idx) +"`");
        }
        int32_t l = open_time(final_line, group_idx), r = close_time(final_line, group_idx);
        if (l < 0 || r < 0) continue;
        for (const auto& captured_char : str.substr(l, r - l)) {
          result_buffer << captured_char;
        }
      } else {
        throw invalid_replacement_rule("invalid argument for `$`");
      }
    }

    return result_buffer.str();
  }

private:
  static constexpr auto pattern_view = impl::FixedStringView<Pattern.length>(Pattern.c_str());
  using Re = typename impl::RegexScan<pattern_view>::type;
  using StateList = impl::tnfa::AllStatesList<Re>;
  using EdgeList  = impl::tnfa::RemoveConflictEdge<impl::tnfa::AllEdgesList<Re>>::type;
  static constexpr std::size_t nr_states = StateList::length;
  static constexpr std::size_t nr_used_slots = impl::tnfa::NrUsedSlots<Re>::value;
  static constexpr std::size_t max_trans_action_length = impl::tnfa::MaxTransActionLength<EdgeList>::value;
  static constexpr std::size_t max_accept_action_length = impl::tnfa::MaxAcceptActionLength<StateList>::value;
  static constexpr std::size_t nr_capture_group = nr_used_slots / 2;

  /* S x Alphabet x S -> bool */
  static constexpr auto trans_table = impl::tnfa::BuildTransTable<nr_states, EdgeList>::make();
  /* S -> bool */
  static constexpr auto accept_table = impl::tnfa::BuildAcceptTable<StateList>::make();
  /* S x Alphabet x S -> int32_t[], -1 represent no action */
  static constexpr auto trans_action_table = impl::tnfa::BuildTransActionTable<nr_states, max_trans_action_length, EdgeList>::make();
  /* S -> int32_t[], -1 represent no action */
  static constexpr auto accept_action_table = impl::tnfa::BuildAcceptActionTable<max_accept_action_length, StateList>::make();

  std::array<bool, nr_states> is_active{};

  using SlotLine = std::array<int32_t, nr_used_slots>;

  static int32_t open_time(SlotLine line, size_t group_idx) { return line[group_idx << 1]; }
  static int32_t close_time(SlotLine line, size_t group_idx) { return line[group_idx << 1 | 1]; }
  static bool is_opened(SlotLine line, size_t group_idx) { return open_time(line, group_idx) >= 0; }
  static bool is_closed(SlotLine line, size_t group_idx) { return close_time(line, group_idx) >= 0; }
  static int32_t group_len(SlotLine line, size_t group_idx) { return close_time(line, group_idx) - open_time(line, group_idx); }

  using SlotFile = std::array<SlotLine, nr_states>;

  static SlotFile new_slot_file() {
    SlotFile res {};
    for (auto& line : res) line.fill(-1);
    return res;
  }

  template<size_t N>
  static SlotLine apply_action(const SlotLine& old_line, const std::array<int32_t, N>& actions, int32_t p) {
    SlotLine new_line{old_line};
    for (const auto& action : actions) {
      if (action < 0) break;
      new_line[action] = p;
    }
    return new_line;
  }

  // heuristically choose a slot configuration to try to get longest match
  static bool need_change(const SlotLine& old_line, const SlotLine& new_line) {
    for (size_t k = 0; k < nr_capture_group; k++) {
      if (!is_opened(old_line, k) && !is_opened(new_line, k)) continue;
      if (is_opened(old_line, k) && !is_opened(new_line, k)) return false;
      if (!is_opened(old_line, k) && is_opened(new_line, k)) return true;
      if (!is_closed(old_line, k) && !is_closed(new_line, k)) {
        if (open_time(old_line, k) < open_time(new_line, k)) return false;
        if (open_time(old_line, k) > open_time(new_line, k)) return true;
        continue;
      }
      if (!is_closed(old_line, k) && is_closed(new_line, k)) return false;
      if (is_closed(old_line, k) && !is_closed(new_line, k)) return true;
      if (group_len(old_line, k) > group_len(new_line, k)) return false;
      if (group_len(old_line, k) < group_len(new_line, k)) return true;
      if (open_time(old_line, k) > open_time(new_line, k)) return false;
      if (open_time(old_line, k) < open_time(new_line, k)) return true;
    }
    return false;
  }

  static bool is_digit(char ch) {
    return '0' <= ch && ch <= '9';
  }
};

} /* namespace onre */

#endif /* !ONRE_REGEX_HPP_ */
