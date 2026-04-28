#ifndef ONRE_REGEX_HPP__
#define ONRE_REGEX_HPP__

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace onre {
namespace impl {

/* === type list, a linear container of types === */
template<typename... Ts>
struct TypeList {
  template<typename T>
  static constexpr bool Contains = (std::is_same_v<T, Ts> || ...);

  template<size_t Idx>
  using At = typename std::tuple_element<Idx, std::tuple<Ts...>>::type;

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
};

template<typename List1, typename List2>
struct CatList;
template<typename... Ts1, typename... Ts2>
struct CatList<TypeList<Ts1...>, TypeList<Ts2...>> {
  using type = TypeList<Ts1..., Ts2...>;
};

template<typename List, typename T>
struct PushFront;
template<typename... Ts, typename T>
struct PushFront<TypeList<Ts...>, T> {
  using type = TypeList<T, Ts...>;
};

template<typename List, typename T>
struct PushBack;
template<typename... Ts, typename T>
struct PushBack<TypeList<Ts...>, T> {
  using type = TypeList<Ts..., T>;
};

template<typename List>
struct PopFront {
  using type = List;
};
template<typename Head, typename... Tails>
struct PopFront<TypeList<Head, Tails...>> {
  using type = TypeList<Tails...>;
};

template <typename List1, typename List2>
struct Join;
template <typename List1, typename Head, typename... Tail>
struct Join<List1, TypeList<Head, Tail...>> {
  using TmpAcc = typename PushBack<List1, Head>::type;
  using type = typename Join<TmpAcc, TypeList<Tail...>>::type;
};
template <typename List1>
struct Join<List1, TypeList<>> {
  using type = List1;
};

template<typename List, typename T>
struct PushBackUnique;
template<typename... Ts, typename T>
struct PushBackUnique<TypeList<Ts...>, T> {
  using type = typename std::conditional<
    TypeList<Ts...>::template Contains<T>,
    TypeList<Ts...>,
    TypeList<Ts..., T>
  >::type;
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

template <typename List>
struct Unique;
template <typename Head, typename... Tails>
struct Unique<TypeList<Head, Tails...>> {
  using type = typename std::conditional<
    TypeList<Tails...>::template Contains<Head>, 
    typename Unique<TypeList<Tails...>>::type,
    typename PushFront<typename Unique<TypeList<Tails...>>::type, Head>::type
  >::type;
};
template<>
struct Unique<TypeList<>> {
  using type = TypeList<>;
};

template<template<typename> typename Func, typename List>
struct Map;
template<template<typename> typename Func, typename... Ts>
struct Map<Func, TypeList<Ts...>> {
  using type = TypeList<typename Func<Ts>::type...>;
};

template<template<typename> typename IsKeep, typename List, typename Acc>
struct FilterImpl;
template<template<typename> typename IsKeep, typename Head, typename... Tails, typename Acc>
struct FilterImpl<IsKeep, TypeList<Head, Tails...>, Acc> {
  using type = typename std::conditional<
    IsKeep<Head>::value,
    FilterImpl<IsKeep, TypeList<Tails...>, typename PushBack<Acc, Head>::type>,
    FilterImpl<IsKeep, TypeList<Tails...>, Acc>
  >::type::type;
};
template<template<typename> typename IsKeep, typename Acc>
struct FilterImpl<IsKeep, TypeList<>, Acc> {
  using type = Acc;
};
template<template<typename> typename IsKeep, typename List>
struct Filter {
  using type = typename FilterImpl<IsKeep, List, TypeList<>>::type;
};

template<template<typename, typename> typename MergeFunc, typename List, typename Begin>
struct RightFold;
template<
  template<typename, typename> typename MergeFunc,
  typename Head,
  typename... Tails,
  typename Begin
>
struct RightFold<MergeFunc, TypeList<Head, Tails...>, Begin> {
  using type = typename MergeFunc<
    Head, typename RightFold<MergeFunc, TypeList<Tails...>, Begin>::type
  >::type;
};
template<template<typename, typename> typename MergeFunc, typename Begin>
struct RightFold<MergeFunc, TypeList<>, Begin> {
  using type = Begin;
};

} /* namespace impl */
} /* namespace onre */

namespace onre {
namespace impl {

/* === fixed string, a string container enabling compile-time visiting === */
template<size_t N>
struct FixedString {
  uint8_t data[N]; /* include '\0' */

  constexpr FixedString(const char (&str)[N]) {
    for (size_t i = 0; i < N; i++)
      data[i] = str[i];
  }
  constexpr FixedString(const char8_t (&str)[N]) {
    for (size_t i = 0; i < N; i++)
      data[i] = str[i];
  }

  constexpr FixedString(const FixedString&) noexcept = default;
  constexpr FixedString(FixedString&&) noexcept = default;
  constexpr FixedString& operator=(const FixedString&) noexcept = default;
  constexpr FixedString& operator=(FixedString&&) noexcept = default;

  static constexpr size_t length = N - 1;
  constexpr const char* c_str() const {
    return (char const*) data; // implementation defined behavior, but on almost all modern system, it works.
  }
  constexpr uint8_t operator[](size_t i) const {
    return data[i];
  }
};
template<size_t N>
FixedString(const char (&str)[N]) -> FixedString<N>;
template<size_t N>
FixedString(const char8_t (&str)[N]) -> FixedString<N>;

} /* namespace impl */
} /* namespace onre */

namespace onre {
namespace impl {

/* === action algebra === */
struct Omega {
  static constexpr size_t length = 0;
};
template<size_t I>
struct Set {
  static constexpr size_t i = I, length = 1;
};
template<typename... As>
struct Seq {
  static constexpr size_t length = (As::length + ...);
};

template<typename A>
struct is_omega : std::false_type {};
template<>
struct is_omega<Omega> : std::true_type {};
template<typename A>
struct is_set : std::false_type {};
template<size_t I>
struct is_set<Set<I>> : std::true_type {};
template<typename A>
struct is_seq : std::false_type {};
template<typename... As>
struct is_seq<Seq<As...>> : std::true_type {};

template<typename A1, typename A2>
struct CatAction;
template<>
struct CatAction<Omega, Omega> {
  using type = Omega;
};
template<size_t I>
struct CatAction<Omega, Set<I>> {
  using type = Set<I>;
};
template<typename... As>
struct CatAction<Omega, Seq<As...>> {
  using type = Seq<As...>;
};
template<size_t I>
struct CatAction<Set<I>, Omega> {
  using type = Set<I>;
};
template<size_t I1, size_t I2>
struct CatAction<Set<I1>, Set<I2>> {
  using type = Seq<Set<I1>, Set<I2>>;
};
template<size_t I, typename... As>
struct CatAction<Set<I>, Seq<As...>> {
  using type = Seq<Set<I>, As...>;
};
template<typename... As>
struct CatAction<Seq<As...>, Omega> {
  using type = Seq<As...>;
};
template<typename... As, size_t I>
struct CatAction<Seq<As...>, Set<I>> {
  using type = Seq<As..., Set<I>>;
};
template<typename... As1, typename... As2>
struct CatAction<Seq<As1...>, Seq<As2...>> {
  using type = Seq<As1..., As2...>;
};
template<typename Seq, typename A> using CarAction_t = typename CatAction<Seq, A>::type;

} /* namespace impl */
} /* namespace onre */

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

using Alphabet = TypeList<
  Char<0x00>, Char<0x01>, Char<0x02>, Char<0x03>, Char<0x04>, Char<0x05>, Char<0x06>, Char<0x07>,
  Char<0x08>, Char<0x09>, Char<0x0A>, Char<0x0B>, Char<0x0C>, Char<0x0D>, Char<0x0E>, Char<0x0F>,
  Char<0x10>, Char<0x11>, Char<0x12>, Char<0x13>, Char<0x14>, Char<0x15>, Char<0x16>, Char<0x17>,
  Char<0x18>, Char<0x19>, Char<0x1A>, Char<0x1B>, Char<0x1C>, Char<0x1D>, Char<0x1E>, Char<0x1F>,
  Char<0x20>, Char<0x21>, Char<0x22>, Char<0x23>, Char<0x24>, Char<0x25>, Char<0x26>, Char<0x27>,
  Char<0x28>, Char<0x29>, Char<0x2A>, Char<0x2B>, Char<0x2C>, Char<0x2D>, Char<0x2E>, Char<0x2F>,
  Char<0x30>, Char<0x31>, Char<0x32>, Char<0x33>, Char<0x34>, Char<0x35>, Char<0x36>, Char<0x37>,
  Char<0x38>, Char<0x39>, Char<0x3A>, Char<0x3B>, Char<0x3C>, Char<0x3D>, Char<0x3E>, Char<0x3F>,
  Char<0x40>, Char<0x41>, Char<0x42>, Char<0x43>, Char<0x44>, Char<0x45>, Char<0x46>, Char<0x47>,
  Char<0x48>, Char<0x49>, Char<0x4A>, Char<0x4B>, Char<0x4C>, Char<0x4D>, Char<0x4E>, Char<0x4F>,
  Char<0x50>, Char<0x51>, Char<0x52>, Char<0x53>, Char<0x54>, Char<0x55>, Char<0x56>, Char<0x57>,
  Char<0x58>, Char<0x59>, Char<0x5A>, Char<0x5B>, Char<0x5C>, Char<0x5D>, Char<0x5E>, Char<0x5F>,
  Char<0x60>, Char<0x61>, Char<0x62>, Char<0x63>, Char<0x64>, Char<0x65>, Char<0x66>, Char<0x67>,
  Char<0x68>, Char<0x69>, Char<0x6A>, Char<0x6B>, Char<0x6C>, Char<0x6D>, Char<0x6E>, Char<0x6F>,
  Char<0x70>, Char<0x71>, Char<0x72>, Char<0x73>, Char<0x74>, Char<0x75>, Char<0x76>, Char<0x77>,
  Char<0x78>, Char<0x79>, Char<0x7A>, Char<0x7B>, Char<0x7C>, Char<0x7D>, Char<0x7E>, Char<0x7F>,
  Char<0x80>, Char<0x81>, Char<0x82>, Char<0x83>, Char<0x84>, Char<0x85>, Char<0x86>, Char<0x87>,
  Char<0x88>, Char<0x89>, Char<0x8A>, Char<0x8B>, Char<0x8C>, Char<0x8D>, Char<0x8E>, Char<0x8F>,
  Char<0x90>, Char<0x91>, Char<0x92>, Char<0x93>, Char<0x94>, Char<0x95>, Char<0x96>, Char<0x97>,
  Char<0x98>, Char<0x99>, Char<0x9A>, Char<0x9B>, Char<0x9C>, Char<0x9D>, Char<0x9E>, Char<0x9F>,
  Char<0xA0>, Char<0xA1>, Char<0xA2>, Char<0xA3>, Char<0xA4>, Char<0xA5>, Char<0xA6>, Char<0xA7>,
  Char<0xA8>, Char<0xA9>, Char<0xAA>, Char<0xAB>, Char<0xAC>, Char<0xAD>, Char<0xAE>, Char<0xAF>,
  Char<0xB0>, Char<0xB1>, Char<0xB2>, Char<0xB3>, Char<0xB4>, Char<0xB5>, Char<0xB6>, Char<0xB7>,
  Char<0xB8>, Char<0xB9>, Char<0xBA>, Char<0xBB>, Char<0xBC>, Char<0xBD>, Char<0xBE>, Char<0xBF>,
  Char<0xC0>, Char<0xC1>, Char<0xC2>, Char<0xC3>, Char<0xC4>, Char<0xC5>, Char<0xC6>, Char<0xC7>,
  Char<0xC8>, Char<0xC9>, Char<0xCA>, Char<0xCB>, Char<0xCC>, Char<0xCD>, Char<0xCE>, Char<0xCF>,
  Char<0xD0>, Char<0xD1>, Char<0xD2>, Char<0xD3>, Char<0xD4>, Char<0xD5>, Char<0xD6>, Char<0xD7>,
  Char<0xD8>, Char<0xD9>, Char<0xDA>, Char<0xDB>, Char<0xDC>, Char<0xDD>, Char<0xDE>, Char<0xDF>,
  Char<0xE0>, Char<0xE1>, Char<0xE2>, Char<0xE3>, Char<0xE4>, Char<0xE5>, Char<0xE6>, Char<0xE7>,
  Char<0xE8>, Char<0xE9>, Char<0xEA>, Char<0xEB>, Char<0xEC>, Char<0xED>, Char<0xEE>, Char<0xEF>,
  Char<0xF0>, Char<0xF1>, Char<0xF2>, Char<0xF3>, Char<0xF4>, Char<0xF5>, Char<0xF6>, Char<0xF7>,
  Char<0xF8>, Char<0xF9>, Char<0xFA>, Char<0xFB>, Char<0xFC>, Char<0xFD>, Char<0xFE>, Char<0xFF>
>;

using Utf8FirstByteAlphabet = TypeList<
  Char<0x00>, Char<0x01>, Char<0x02>, Char<0x03>, Char<0x04>, Char<0x05>, Char<0x06>, Char<0x07>,
  Char<0x08>, Char<0x09>, Char<0x0A>, Char<0x0B>, Char<0x0C>, Char<0x0D>, Char<0x0E>, Char<0x0F>,
  Char<0x10>, Char<0x11>, Char<0x12>, Char<0x13>, Char<0x14>, Char<0x15>, Char<0x16>, Char<0x17>,
  Char<0x18>, Char<0x19>, Char<0x1A>, Char<0x1B>, Char<0x1C>, Char<0x1D>, Char<0x1E>, Char<0x1F>,
  Char<0x20>, Char<0x21>, Char<0x22>, Char<0x23>, Char<0x24>, Char<0x25>, Char<0x26>, Char<0x27>,
  Char<0x28>, Char<0x29>, Char<0x2A>, Char<0x2B>, Char<0x2C>, Char<0x2D>, Char<0x2E>, Char<0x2F>,
  Char<0x30>, Char<0x31>, Char<0x32>, Char<0x33>, Char<0x34>, Char<0x35>, Char<0x36>, Char<0x37>,
  Char<0x38>, Char<0x39>, Char<0x3A>, Char<0x3B>, Char<0x3C>, Char<0x3D>, Char<0x3E>, Char<0x3F>,
  Char<0x40>, Char<0x41>, Char<0x42>, Char<0x43>, Char<0x44>, Char<0x45>, Char<0x46>, Char<0x47>,
  Char<0x48>, Char<0x49>, Char<0x4A>, Char<0x4B>, Char<0x4C>, Char<0x4D>, Char<0x4E>, Char<0x4F>,
  Char<0x50>, Char<0x51>, Char<0x52>, Char<0x53>, Char<0x54>, Char<0x55>, Char<0x56>, Char<0x57>,
  Char<0x58>, Char<0x59>, Char<0x5A>, Char<0x5B>, Char<0x5C>, Char<0x5D>, Char<0x5E>, Char<0x5F>,
  Char<0x60>, Char<0x61>, Char<0x62>, Char<0x63>, Char<0x64>, Char<0x65>, Char<0x66>, Char<0x67>,
  Char<0x68>, Char<0x69>, Char<0x6A>, Char<0x6B>, Char<0x6C>, Char<0x6D>, Char<0x6E>, Char<0x6F>,
  Char<0x70>, Char<0x71>, Char<0x72>, Char<0x73>, Char<0x74>, Char<0x75>, Char<0x76>, Char<0x77>,
  Char<0x78>, Char<0x79>, Char<0x7A>, Char<0x7B>, Char<0x7C>, Char<0x7D>, Char<0x7E>, Char<0x7F>,
  Char<0xC2>, Char<0xC3>, Char<0xC4>, Char<0xC5>, Char<0xC6>, Char<0xC7>, Char<0xC8>, Char<0xC9>,
  Char<0xCA>, Char<0xCB>, Char<0xCC>, Char<0xCD>, Char<0xCE>, Char<0xCF>, Char<0xD0>, Char<0xD1>,
  Char<0xD2>, Char<0xD3>, Char<0xD4>, Char<0xD5>, Char<0xD6>, Char<0xD7>, Char<0xD8>, Char<0xD9>,
  Char<0xDA>, Char<0xDB>, Char<0xDC>, Char<0xDD>, Char<0xDE>, Char<0xDF>, Char<0xE0>, Char<0xE1>,
  Char<0xE2>, Char<0xE3>, Char<0xE4>, Char<0xE5>, Char<0xE6>, Char<0xE7>, Char<0xE8>, Char<0xE9>,
  Char<0xEA>, Char<0xEB>, Char<0xEC>, Char<0xED>, Char<0xEE>, Char<0xEF>, Char<0xF0>, Char<0xF1>,
  Char<0xF2>, Char<0xF3>, Char<0xF4>
>;
using Utf8ContinuationByteAlphabet = TypeList<
  Char<0x80>, Char<0x81>, Char<0x82>, Char<0x83>, Char<0x84>, Char<0x85>, Char<0x86>, Char<0x87>,
  Char<0x88>, Char<0x89>, Char<0x8A>, Char<0x8B>, Char<0x8C>, Char<0x8D>, Char<0x8E>, Char<0x8F>,
  Char<0x90>, Char<0x91>, Char<0x92>, Char<0x93>, Char<0x94>, Char<0x95>, Char<0x96>, Char<0x97>,
  Char<0x98>, Char<0x99>, Char<0x9A>, Char<0x9B>, Char<0x9C>, Char<0x9D>, Char<0x9E>, Char<0x9F>,
  Char<0xA0>, Char<0xA1>, Char<0xA2>, Char<0xA3>, Char<0xA4>, Char<0xA5>, Char<0xA6>, Char<0xA7>,
  Char<0xA8>, Char<0xA9>, Char<0xAA>, Char<0xAB>, Char<0xAC>, Char<0xAD>, Char<0xAE>, Char<0xAF>,
  Char<0xB0>, Char<0xB1>, Char<0xB2>, Char<0xB3>, Char<0xB4>, Char<0xB5>, Char<0xB6>, Char<0xB7>,
  Char<0xB8>, Char<0xB9>, Char<0xBA>, Char<0xBB>, Char<0xBC>, Char<0xBD>, Char<0xBE>, Char<0xBF>
>;

} /* namespace impl */
} /* namespace onre */

namespace onre {
namespace impl {

/* === extended regular expression tree representation with zero-width action === */
struct EmptySet {};
struct Epsilon {};
template<uint8_t C>
struct Char {
  static constexpr uint8_t c = C;
};
struct Wildcard {};
struct Wildcard3 {};
struct Wildcard2 {};
struct Wildcard1 {};
template<typename CharList>
struct Except {};
template<std::size_t I>
struct SetSlot {
  static constexpr std::size_t i = I;
};
template<typename R, typename S>
struct Or {
  using left = R; using right = S;
};
template<typename R, typename S>
struct Concat {
  using left = R; using right = S;
};
template<typename R>
struct Closure {
  using inner = R;
};

/* === nullable testing, testing whether epsilon in L(R) === */
template<typename R>
struct Nullable : std::false_type {};
template<>
struct Nullable<Epsilon> : std::true_type {};
template<size_t I>
struct Nullable<SetSlot<I>> : std::true_type {};
template<typename L, typename R>
struct Nullable<Or<L, R>>
  : std::bool_constant<Nullable<L>::value || Nullable<R>::value> {};
template<typename L, typename R>
struct Nullable<Concat<L, R>>
  : std::bool_constant<Nullable<L>::value && Nullable<R>::value> {};
template<typename R>
struct Nullable<Closure<R>> : std::true_type {};

/*                        First notation,                           *
 * === get TypeList<Char...> of possible character occurring in === *
 *               the head of string in RE language                  */

template <typename RE, typename Acc>
struct First;
template <typename Acc>
struct First<EmptySet, Acc> {
  using type = Acc;
};
template <typename Acc>
struct First<Epsilon, Acc> {
  using type = Acc;
};
template <uint8_t C, typename Acc>
struct First<Char<C>, Acc> {
  using type = typename PushBackUnique<Acc, Char<C>>::type;
};
template<typename Acc>
struct First<Wildcard, Acc> {
  using NotDotAllWildcardAlphabet = TypeList<
    Char<0x00>, Char<0x01>, Char<0x02>, Char<0x03>, Char<0x04>, Char<0x05>, Char<0x06>, Char<0x07>,
    Char<0x08>, Char<0x09>, Char<0x0B>, Char<0x0C>, Char<0x0E>, Char<0x0F>, Char<0x10>, Char<0x11>,
    Char<0x12>, Char<0x13>, Char<0x14>, Char<0x15>, Char<0x16>, Char<0x17>, Char<0x18>, Char<0x19>,
    Char<0x1A>, Char<0x1B>, Char<0x1C>, Char<0x1D>, Char<0x1E>, Char<0x1F>, Char<0x20>, Char<0x21>,
    Char<0x22>, Char<0x23>, Char<0x24>, Char<0x25>, Char<0x26>, Char<0x27>, Char<0x28>, Char<0x29>,
    Char<0x2A>, Char<0x2B>, Char<0x2C>, Char<0x2D>, Char<0x2E>, Char<0x2F>, Char<0x30>, Char<0x31>,
    Char<0x32>, Char<0x33>, Char<0x34>, Char<0x35>, Char<0x36>, Char<0x37>, Char<0x38>, Char<0x39>,
    Char<0x3A>, Char<0x3B>, Char<0x3C>, Char<0x3D>, Char<0x3E>, Char<0x3F>, Char<0x40>, Char<0x41>,
    Char<0x42>, Char<0x43>, Char<0x44>, Char<0x45>, Char<0x46>, Char<0x47>, Char<0x48>, Char<0x49>,
    Char<0x4A>, Char<0x4B>, Char<0x4C>, Char<0x4D>, Char<0x4E>, Char<0x4F>, Char<0x50>, Char<0x51>,
    Char<0x52>, Char<0x53>, Char<0x54>, Char<0x55>, Char<0x56>, Char<0x57>, Char<0x58>, Char<0x59>,
    Char<0x5A>, Char<0x5B>, Char<0x5C>, Char<0x5D>, Char<0x5E>, Char<0x5F>, Char<0x60>, Char<0x61>,
    Char<0x62>, Char<0x63>, Char<0x64>, Char<0x65>, Char<0x66>, Char<0x67>, Char<0x68>, Char<0x69>,
    Char<0x6A>, Char<0x6B>, Char<0x6C>, Char<0x6D>, Char<0x6E>, Char<0x6F>, Char<0x70>, Char<0x71>,
    Char<0x72>, Char<0x73>, Char<0x74>, Char<0x75>, Char<0x76>, Char<0x77>, Char<0x78>, Char<0x79>,
    Char<0x7A>, Char<0x7B>, Char<0x7C>, Char<0x7D>, Char<0x7E>, Char<0x7F>, Char<0xC2>, Char<0xC3>,
    Char<0xC4>, Char<0xC5>, Char<0xC6>, Char<0xC7>, Char<0xC8>, Char<0xC9>, Char<0xCA>, Char<0xCB>,
    Char<0xCC>, Char<0xCD>, Char<0xCE>, Char<0xCF>, Char<0xD0>, Char<0xD1>, Char<0xD2>, Char<0xD3>,
    Char<0xD4>, Char<0xD5>, Char<0xD6>, Char<0xD7>, Char<0xD8>, Char<0xD9>, Char<0xDA>, Char<0xDB>,
    Char<0xDC>, Char<0xDD>, Char<0xDE>, Char<0xDF>, Char<0xE0>, Char<0xE1>, Char<0xE2>, Char<0xE3>,
    Char<0xE4>, Char<0xE5>, Char<0xE6>, Char<0xE7>, Char<0xE8>, Char<0xE9>, Char<0xEA>, Char<0xEB>,
    Char<0xEC>, Char<0xED>, Char<0xEE>, Char<0xEF>, Char<0xF0>, Char<0xF1>, Char<0xF2>, Char<0xF3>,
    Char<0xF4>
  >;

  #ifdef ONRE_DOTALL
    using WildcardAlphabet = Utf8FirstByteAlphabet;
  #else
    using WildcardAlphabet = NotDotAllWildcardAlphabet;
  #endif

  using type = typename JoinUnique<Acc, WildcardAlphabet>::type;
};
template<typename Acc>
struct First<Wildcard3, Acc> {
  using type = typename JoinUnique<Acc, Utf8ContinuationByteAlphabet>::type;
};
template<typename Acc>
struct First<Wildcard2, Acc> {
  using type = typename JoinUnique<Acc, Utf8ContinuationByteAlphabet>::type;
};
template<typename Acc>
struct First<Wildcard1, Acc> {
  using type = typename JoinUnique<Acc, Utf8ContinuationByteAlphabet>::type;
};
template <typename CharList, typename Acc>
struct First<Except<CharList>, Acc> {
  using type = typename JoinUnique<
    Acc,
    typename CharListNegation<CharList, Utf8FirstByteAlphabet>::type
  >::type;
};
template <size_t I, typename Acc>
struct First<SetSlot<I>, Acc> {
  using type = Acc;
};
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
  using type = typename std::conditional<
    Nullable<R>::value,
    impl_r_nullable,
    impl_r_non_nullable
  >::type::type;
};
template <typename R, typename Acc>
struct First<Closure<R>, Acc> {
  using type = typename First<R, Acc>::type;
};

} /* namespace impl */
} /* namespace onre */

namespace onre {
namespace impl {

/* === simplify and standard ordering rules, significantly reduce complexity === */
template<typename R>
struct Simplify {
  using type = R;
};
/* 0|R <=> R|0 <=> R */
template<typename R>
struct Simplify<Or<EmptySet, R>> {
  using type = typename Simplify<R>::type;
};
template<typename R>
struct Simplify<Or<R, EmptySet>> {
  using type = typename Simplify<R>::type;
};
/* R|R <=> R */
template<typename T>
struct Simplify<Or<T, T>> {
  using type = typename Simplify<T>::type;
};
template<>
struct Simplify<Or<EmptySet, EmptySet>> {
  using type = EmptySet;
};
/* .|c <=> c|. <=> .*/
template<uint8_t C>
struct Simplify<Or<Wildcard, Char<C>>> {
  using type = Wildcard;
};
template<uint8_t C>
struct Simplify<Or<Char<C>, Wildcard>> {
  using type = Wildcard;
};
// /* .*|R <=> R|.* <=> .* */
template<typename R>
struct Simplify<Or<Closure<Wildcard>, R>> {
  using type = Closure<Wildcard>;
};
template<typename R>
struct Simplify<Or<R, Closure<Wildcard>>> {
  using type = Closure<Wildcard>;
};
template<>
struct Simplify<Or<Closure<Wildcard>, EmptySet>> {
  using type = Closure<Wildcard>;
};
template<>
struct Simplify<Or<EmptySet, Closure<Wildcard>>> {
  using type = Closure<Wildcard>;
};
template<>
struct Simplify<Or<Closure<Wildcard>, Closure<Wildcard>>> {
  using type = Closure<Wildcard>;
};
/* !list|a <=> a|!list <=> !list iff. a not in list*/
template<typename CharList, uint8_t C>
struct Simplify<Or<Except<CharList>, Char<C>>> {
  using type = typename std::conditional<
    CharList::template Contains<Char<C>>,
    Or<Except<CharList>, Char<C>>,
    Except<CharList>
  >::type;
};
template<typename CharList, uint8_t C>
struct Simplify<Or<Char<C>, Except<CharList>>> {
  using type = typename std::conditional<
    CharList::template Contains<Char<C>>,
    Or<Except<CharList>, Char<C>>,
    Except<CharList>
  >::type;
};
/* TR|TS <=> T(R|S) */
template<typename T, typename R, typename S>
struct Simplify<Or<Concat<T, R>, Concat<T, S>>> {
  using type = typename Simplify<Concat<T, Or<R, S>>>::type;
};
template<typename T, typename R>
struct Simplify<Or<Concat<T, R>, Concat<T, R>>> {
  using type = typename Simplify<Concat<T, R>>::type;
};
/* 0R <=> R0 <=> 0 */
template<typename R>
struct Simplify<Concat<EmptySet, R>> {
  using type = EmptySet;
};
template<typename L>
struct Simplify<Concat<L, EmptySet>> {
  using type = EmptySet;
};
template<>
struct Simplify<Concat<EmptySet, EmptySet>> {
  using type = EmptySet;
};
/* eR <=> Re <=> R */
template<typename R>
struct Simplify<Concat<Epsilon, R>> {
  using type = typename Simplify<R>::type;
};
template<typename L>
struct Simplify<Concat<L, Epsilon>> {
  using type = typename Simplify<L>::type;
};
template<>
struct Simplify<Concat<Epsilon, EmptySet>> {
  using type = EmptySet;
};
template<>
struct Simplify<Concat<EmptySet, Epsilon>> {
  using type = EmptySet;
};
template<>
struct Simplify<Concat<Epsilon, Epsilon>> {
  using type = Epsilon;
};

/* e* <=> e */
template<>
struct Simplify<Closure<Epsilon>> {
  using type = Epsilon;
};
/* 0* <=> e */
template<>
struct Simplify<Closure<EmptySet>> {
  using type = Epsilon;
};

/* R** <=> R* */
template<typename R>
struct Simplify<Closure<Closure<R>>> {
  using type = typename Simplify<Closure<R>>::type;
};

/* (e|R)* <=> (R|e)* <=> R* */
template<typename R>
struct Simplify<Closure<Or<Epsilon, R>>> {
  using type = typename Simplify<Closure<R>>::type;
};
template<typename R>
struct Simplify<Closure<Or<R, Epsilon>>> {
  using type = typename Simplify<Closure<R>>::type;
};
template<>
struct Simplify<Closure<Or<Epsilon, Epsilon>>> {
  using type = Epsilon;
};
/* e|RR* <=> RR*|e <=> R* */
template<typename R>
struct Simplify<Or<Epsilon, Concat<R, Closure<R>>>> {
  using type = typename Simplify<Closure<R>>::type;
};
template<typename R>
struct Simplify<Or<Concat<R, Closure<R>>, Epsilon>> {
  using type = typename Simplify<Closure<R>>::type;
};
/* RR*|R* <=> R*R|R <=> R*|RR* <=> R*|R*R <=> RR* */
template<typename R>
struct Simplify<Or<Concat<R, Closure<R>>, Closure<R>>> {
  using type = typename Simplify<Concat<R, Closure<R>>>::type;
};
template<typename R>
struct Simplify<Or<Concat<Closure<R>, R>, Closure<R>>> {
  using type = typename Simplify<Concat<R, Closure<R>>>::type;
};
template<typename R>
struct Simplify<Or<Closure<R>, Concat<R, Closure<R>>>> {
  using type = typename Simplify<Concat<R, Closure<R>>>::type;
};
template<typename R>
struct Simplify<Or<Closure<R>, Concat<Closure<R>, R>>> {
  using type = typename Simplify<Concat<R, Closure<R>>>::type;
};

template<typename R, typename S, typename T>
struct Simplify<Or<Or<R, S>, T>> {
  using type = typename Simplify<Or<R, Or<S, T>>>::type;
};
template<typename R, typename S>
struct Simplify<Or<Or<R, S>, Or<R, S>>> {
  using type = typename Simplify<Or<R, S>>::type;
};
template<typename R, typename S>
struct Simplify<Or<Or<R, S>, EmptySet>> {
  using type = typename Simplify<Or<R, S>>::type;
};
template<typename R, typename S, typename T>
struct Simplify<Concat<Concat<R, S>, T>> {
  using type = typename Simplify<Concat<R, Concat<S, T>>>::type;
};
template<typename R, typename S>
struct Simplify<Concat<Concat<R, S>, EmptySet>> {
  using type = EmptySet;
};
template<typename R, typename S>
struct Simplify<Concat<Concat<R, S>, Epsilon>> {
  using type = typename Simplify<Concat<R, S>>::type;
};
template<typename R>
struct Simplify<Concat<Closure<R>, R>> {
  using type = typename Simplify<Concat<R, Closure<R>>>::type;
};
template<typename R>
struct Simplify<Concat<Closure<R>, Epsilon>> {
  using type = typename Simplify<Closure<R>>::type;
};
template<typename R>
struct Simplify<Concat<Closure<R>, EmptySet>> {
  using type = EmptySet;
};

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

} /* namespace impl */
} /* namespace onre */

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
    [[maybe_unused]] auto existing_len_of = []<size_t L>(const std::array<int32_t, L>& list) constexpr {
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

template<typename Char>
struct CharToCodePoint {
  using type = CodePoint<Char::c>;
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

template<typename CodePointList>
struct BuildOrTree {
  template<typename C1, typename C2>
  struct MergeF { using type = Or<C1, C2>; };
  template <typename CodePoint>
  struct MapF { using type = typename BuildUtf8ByteStreamRegex<CodePoint::value>::type; };
  using type = typename Simplify<typename RightFold<
    MergeF,
    typename Map<MapF, CodePointList>::type,
    EmptySet
  >::type>::type;
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
  using chosen = typename std::conditional<
    Pos < Pattern.length && Pattern[Pos] >= '0' && Pattern[Pos] <= '9',
    is_digit_impl,
    not_digit_impl
  >::type;
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
  using chosen = typename std::conditional<
    (N > 0) && Pos < Pattern.length && (
      (Pattern[Pos] >= '0' && Pattern[Pos] <= '9')
      || (Pattern[Pos] >= 'a' && Pattern[Pos] <= 'f')
      || (Pattern[Pos] >= 'A' && Pattern[Pos] <= 'F')
    ),
    is_digit_impl,
    not_digit_impl
  >::type;
  static constexpr int64_t value = chosen::value;
  static constexpr size_t next = chosen::next;
};

template<uint8_t C, FixedString Pattern, size_t Pos>
struct EscapeImpl {
  using type = TypeList<CodePoint<Pattern[Pos + 1]>>;
  static constexpr size_t next = Pos + 2;
};
using WordList = TypeList<
  CodePoint<'a'>, CodePoint<'b'>, CodePoint<'c'>, CodePoint<'d'>, CodePoint<'e'>, CodePoint<'f'>,
  CodePoint<'g'>, CodePoint<'h'>, CodePoint<'i'>, CodePoint<'j'>, CodePoint<'k'>, CodePoint<'l'>,
  CodePoint<'m'>, CodePoint<'n'>, CodePoint<'o'>, CodePoint<'p'>, CodePoint<'q'>, CodePoint<'r'>,
  CodePoint<'s'>, CodePoint<'t'>, CodePoint<'u'>, CodePoint<'v'>, CodePoint<'w'>, CodePoint<'x'>,
  CodePoint<'y'>, CodePoint<'z'>, CodePoint<'A'>, CodePoint<'B'>, CodePoint<'C'>, CodePoint<'D'>,
  CodePoint<'E'>, CodePoint<'F'>, CodePoint<'G'>, CodePoint<'H'>, CodePoint<'I'>, CodePoint<'J'>,
  CodePoint<'K'>, CodePoint<'L'>, CodePoint<'M'>, CodePoint<'N'>, CodePoint<'O'>, CodePoint<'P'>,
  CodePoint<'Q'>, CodePoint<'R'>, CodePoint<'S'>, CodePoint<'T'>, CodePoint<'U'>, CodePoint<'V'>,
  CodePoint<'W'>, CodePoint<'X'>, CodePoint<'Y'>, CodePoint<'Z'>, CodePoint<'0'>, CodePoint<'1'>,
  CodePoint<'2'>, CodePoint<'3'>, CodePoint<'4'>, CodePoint<'5'>, CodePoint<'6'>, CodePoint<'7'>,
  CodePoint<'8'>, CodePoint<'9'>, CodePoint<'_'>
>;
using NegativeWordList = TypeList<
  CodePoint<'\t'>, CodePoint<'\n'>, CodePoint<'\v'>, CodePoint<'\f'>, CodePoint<'\r'>, CodePoint<' '>,
  CodePoint<'!'>, CodePoint<'"'>, CodePoint<'#'>, CodePoint<'$'>, CodePoint<'%'>, CodePoint<'&'>,
  CodePoint<'\''>, CodePoint<'('>, CodePoint<')'>, CodePoint<'*'>, CodePoint<'+'>, CodePoint<','>,
  CodePoint<'-'>, CodePoint<'.'>, CodePoint<'/'>, CodePoint<':'>, CodePoint<';'>, CodePoint<'<'>,
  CodePoint<'='>, CodePoint<'>'>, CodePoint<'?'>, CodePoint<'@'>, CodePoint<'['>, CodePoint<'\\'>,
  CodePoint<']'>, CodePoint<'^'>, CodePoint<'`'>, CodePoint<'{'>, CodePoint<'|'>, CodePoint<'}'>, CodePoint<'~'>
>;
using DigitalList = TypeList<
  CodePoint<'0'>, CodePoint<'1'>, CodePoint<'2'>, CodePoint<'3'>, CodePoint<'4'>,
  CodePoint<'5'>, CodePoint<'6'>, CodePoint<'7'>, CodePoint<'8'>, CodePoint<'9'>
>;
using NegativeDigitalList = TypeList<
  CodePoint<'\t'>, CodePoint<'\n'>, CodePoint<'\v'>, CodePoint<'\f'>, CodePoint<'\r'>, CodePoint<' '>,
  CodePoint<'!'>, CodePoint<'"'>, CodePoint<'#'>, CodePoint<'$'>, CodePoint<'%'>, CodePoint<'&'>,
  CodePoint<'\''>, CodePoint<'('>, CodePoint<')'>, CodePoint<'*'>, CodePoint<'+'>, CodePoint<','>,
  CodePoint<'-'>, CodePoint<'.'>, CodePoint<'/'>, CodePoint<':'>, CodePoint<';'>, CodePoint<'<'>,
  CodePoint<'='>, CodePoint<'>'>, CodePoint<'?'>, CodePoint<'@'>, CodePoint<'A'>, CodePoint<'B'>,
  CodePoint<'C'>, CodePoint<'D'>, CodePoint<'E'>, CodePoint<'F'>, CodePoint<'G'>, CodePoint<'H'>,
  CodePoint<'I'>, CodePoint<'J'>, CodePoint<'K'>, CodePoint<'L'>, CodePoint<'M'>, CodePoint<'N'>,
  CodePoint<'O'>, CodePoint<'P'>, CodePoint<'Q'>, CodePoint<'R'>, CodePoint<'S'>, CodePoint<'T'>,
  CodePoint<'U'>, CodePoint<'V'>, CodePoint<'W'>, CodePoint<'X'>, CodePoint<'Y'>, CodePoint<'Z'>,
  CodePoint<'['>, CodePoint<'\\'>, CodePoint<']'>, CodePoint<'^'>, CodePoint<'_'>, CodePoint<'`'>,
  CodePoint<'a'>, CodePoint<'b'>, CodePoint<'c'>, CodePoint<'d'>, CodePoint<'e'>, CodePoint<'f'>,
  CodePoint<'g'>, CodePoint<'h'>, CodePoint<'i'>, CodePoint<'j'>, CodePoint<'k'>, CodePoint<'l'>,
  CodePoint<'m'>, CodePoint<'n'>, CodePoint<'o'>, CodePoint<'p'>, CodePoint<'q'>, CodePoint<'r'>,
  CodePoint<'s'>, CodePoint<'t'>, CodePoint<'u'>, CodePoint<'v'>, CodePoint<'w'>, CodePoint<'x'>,
  CodePoint<'y'>, CodePoint<'z'>, CodePoint<'{'>, CodePoint<'|'>, CodePoint<'}'>, CodePoint<'~'>
>;
using WhitespaceList = TypeList<
  CodePoint<'\t'>, CodePoint<'\n'>, CodePoint<'\v'>, CodePoint<'\f'>, CodePoint<'\r'>, CodePoint<' '>
>;
using NegativeWhitespaceList = TypeList<
  CodePoint<'!'>, CodePoint<'"'>, CodePoint<'#'>, CodePoint<'$'>, CodePoint<'%'>, CodePoint<'&'>,
  CodePoint<'\''>, CodePoint<'('>, CodePoint<')'>, CodePoint<'*'>, CodePoint<'+'>, CodePoint<','>,
  CodePoint<'-'>, CodePoint<'.'>, CodePoint<'/'>, CodePoint<'0'>, CodePoint<'1'>, CodePoint<'2'>,
  CodePoint<'3'>, CodePoint<'4'>, CodePoint<'5'>, CodePoint<'6'>, CodePoint<'7'>, CodePoint<'8'>,
  CodePoint<'9'>, CodePoint<':'>, CodePoint<';'>, CodePoint<'<'>, CodePoint<'='>, CodePoint<'>'>,
  CodePoint<'?'>, CodePoint<'@'>, CodePoint<'A'>, CodePoint<'B'>, CodePoint<'C'>, CodePoint<'D'>,
  CodePoint<'E'>, CodePoint<'F'>, CodePoint<'G'>, CodePoint<'H'>, CodePoint<'I'>, CodePoint<'J'>,
  CodePoint<'K'>, CodePoint<'L'>, CodePoint<'M'>, CodePoint<'N'>, CodePoint<'O'>, CodePoint<'P'>,
  CodePoint<'Q'>, CodePoint<'R'>, CodePoint<'S'>, CodePoint<'T'>, CodePoint<'U'>, CodePoint<'V'>,
  CodePoint<'W'>, CodePoint<'X'>, CodePoint<'Y'>, CodePoint<'Z'>, CodePoint<'['>, CodePoint<'\\'>,
  CodePoint<']'>, CodePoint<'^'>, CodePoint<'_'>, CodePoint<'`'>, CodePoint<'a'>, CodePoint<'b'>,
  CodePoint<'c'>, CodePoint<'d'>, CodePoint<'e'>, CodePoint<'f'>, CodePoint<'g'>, CodePoint<'h'>,
  CodePoint<'i'>, CodePoint<'j'>, CodePoint<'k'>, CodePoint<'l'>, CodePoint<'m'>, CodePoint<'n'>,
  CodePoint<'o'>, CodePoint<'p'>, CodePoint<'q'>, CodePoint<'r'>, CodePoint<'s'>, CodePoint<'t'>,
  CodePoint<'u'>, CodePoint<'v'>, CodePoint<'w'>, CodePoint<'x'>, CodePoint<'y'>, CodePoint<'z'>,
  CodePoint<'{'>, CodePoint<'|'>, CodePoint<'}'>, CodePoint<'~'>
>;
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'w', Pattern, Pos> {
  using type = WordList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'W', Pattern, Pos> {
  using type = NegativeWordList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'d', Pattern, Pos> {
  using type = DigitalList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'D', Pattern, Pos> {
  using type = NegativeDigitalList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'s', Pattern, Pos> {
  using type = WhitespaceList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'S', Pattern, Pos> {
  using type = NegativeWhitespaceList;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'n', Pattern, Pos> {
  using type = TypeList<CodePoint<'\n'>>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'t', Pattern, Pos> {
  using type = TypeList<CodePoint<'\t'>>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'f', Pattern, Pos> {
  using type = TypeList<CodePoint<'\f'>>;
  static constexpr size_t next = Pos + 2;
};
template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'r', Pattern, Pos> {
  using type = TypeList<CodePoint<'\r'>>;
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
  using type = TypeList<CodePoint<HexParse::value>>;
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

  using chosen = typename std::conditional<
    Pattern[Pos] == '}',
    impl_end,
    impl_digit
  >::type;
  static constexpr uint32_t value = chosen::value;
  static constexpr size_t next = chosen::next;
  static_assert(value <= 0x10FFFF, "ParseEscape: Unicode code point overflow");
};

template<FixedString Pattern, size_t Pos>
struct EscapeImpl<'u', Pattern, Pos> {
  static_assert(Pos + 3 < Pattern.length, "ParseEscape: incomplete Unicode escape");
  static_assert(Pattern[Pos + 2] == '{', "ParseEscape: malformed Unicode escape");
  using HexParse = ParseHexBraced<Pattern, Pos + 3, 0, 0>;
  using type = TypeList<CodePoint<HexParse::value>>;
  static constexpr size_t next = HexParse::next;
};

template <FixedString Pattern, size_t Pos>
struct ParseEscape {
  static_assert(Pos + 1 < Pattern.length, "ParseEscape: cannot find escape character");
  using chosen = EscapeImpl<Pattern[Pos + 1], Pattern, Pos>;
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
    using type = typename BuildOrTree<typename EscapeParse::type>::type;
    static constexpr size_t next = EscapeParse::next;
  };

  struct impl_simple {
    using CodePoint = ParseUtf8CodePoint<Pattern, Pos>;
    using type = typename BuildUtf8ByteStreamRegex<CodePoint::value>::type;
    static constexpr size_t next = CodePoint::next;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  using chosen = typename std::conditional<is_escape, impl_escape, impl_simple>::type;
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
    : (length == 3) ? static_cast<uint8_t>(0x80 | ((Code >> 6) & 0x3F))
    : static_cast<uint8_t>(0x80 | ((Code >> 12) & 0x3F));
  static constexpr uint8_t b2 =
    (length <= 2) ? 0
    : (length == 3) ? static_cast<uint8_t>(0x80 | (Code & 0x3F))
    : static_cast<uint8_t>(0x80 | ((Code >> 6) & 0x3F));
  static constexpr uint8_t b3 =
    (length <= 3) ? 0
    : static_cast<uint8_t>(0x80 | (Code & 0x3F));
};

/* ParseCharSetAtom: [IN CLASS CHAR] | [IN CLASS CHAR] '-' [IN CLASS CHAR] | Escape */
template<FixedString Pattern, size_t Pos>
struct ParseCharSetAtom {
  static_assert(Pos < Pattern.length, "ParseCharSetAtom: unexpected pattern ending");
  static_assert(is_in_class_char(Pattern[Pos]), "ParseCharSetAtom: unknown character");

  struct impl_char {
    using CodePointParse = ParseUtf8CodePoint<Pattern, Pos>;
    using type = TypeList<CodePoint<CodePointParse::value>>;
    static constexpr size_t next = CodePointParse::next;
  };

  struct impl_seq {
    using Start = ParseUtf8CodePoint<Pattern, Pos>;
    static_assert(Start::next + 1 < Pattern.length, "ParseCharSetAtom: `-` has no ending");
    using End = ParseUtf8CodePoint<Pattern, Start::next + 1>;
    using type = typename BuildCodePointRange<Start::value, End::value>::type;
    static constexpr size_t next = End::next;
  };

  struct impl_escape {
    static_assert(Pos + 1 < Pattern.length, "unexpected ending");
    using EscapeParse = ParseEscape<Pattern, Pos>;
    using type = typename EscapeParse::type;
    static constexpr size_t next = EscapeParse::next;
  };

  static constexpr bool is_escape = Pattern[Pos] == '\\';
  static constexpr bool has_hyphen = !is_escape
    && ParseUtf8CodePoint<Pattern, Pos>::next < Pattern.length
    && Pattern[ParseUtf8CodePoint<Pattern, Pos>::next] == '-';
  using chosen = typename std::conditional<
    is_escape,
    impl_escape,
    typename std::conditional<has_hyphen, impl_seq, impl_char>::type
  >::type;
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
    using type = typename Join<typename CharSetAtom::type, typename Next::type>::type;
    static constexpr size_t next = Next::next;
  };

  struct impl_stop {
    using type = typename CharSetAtom::type;
    static constexpr size_t next = CharSetAtom::next;
  };

  static constexpr bool run_on = CharSetAtom::next < Pattern.length
    && is_in_class_char(Pattern[CharSetAtom::next]);
  using chosen = typename std::conditional<run_on, impl_run_on, impl_stop>::type;
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
    using type = typename BuildOrTree<typename Unique<typename CharSet::type>::type>::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  struct impl_neg {
    using CharSet = ParseCharSet<Pattern, Pos + 2>;
    static_assert(CharSet::next <= Pattern.length, "ParseCharGroup: char set parsing overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']',
      "ParseCharGroup: ']' not closed");
    template <typename CodePoint> 
    struct CodePointToChar{
      static_assert(CodePoint::value <= 0x7F, "negation char class do not support non-ASCII so far");
      using type = Char<static_cast<uint8_t>(CodePoint::value)>;
    };
    using type = Except<typename Map<CodePointToChar, typename CharSet::type>::type>;
    static constexpr size_t next = CharSet::next + 1;
  };

  static constexpr bool is_neg = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '^';
  using chosen = typename std::conditional<is_neg, impl_neg, impl_pos>::type;
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
      using type = typename std::conditional<
        is_non_capturing,
        typename Regex::type,
        Concat<
          SetSlot<2 * CapIdx>,
          Concat<typename Regex::type, SetSlot<2 * CapIdx + 1>>
        >
      >::type;
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

  using chosen = typename std::conditional<
    Pattern[Pos] == '(',
    impl_paren,
    typename std::conditional<
      Pattern[Pos] == '[',
      impl_square,
      typename std::conditional<
        Pattern[Pos] == '.',
        impl_full_match,
        impl_char
      >::type
    >::type
  >::type;
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
  using type = typename std::conditional<Max < 0, inf, non_inf>::type::type;
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

    using chosen = typename std::conditional<Pattern[ParseMin::next] == '}', single_num, multiple_num>::type;
    static constexpr int64_t Max = chosen::Max;

    static_assert(Max < 0 || Min <= Max, "ParseFactor: invalid quantifier");

    using type = typename BuildQuantifier<typename Atom::type, Min, Max>::type;
    static constexpr size_t next = chosen::next;
  };

  static constexpr bool has_star = (Atom::next < Pattern.length && Pattern[Atom::next] == '*');
  static constexpr bool has_plus = (Atom::next < Pattern.length && Pattern[Atom::next] == '+');
  static constexpr bool has_question = (Atom::next < Pattern.length && Pattern[Atom::next] == '?');
  static constexpr bool has_curly = (Atom::next < Pattern.length && Pattern[Atom::next] == '{');

  using chosen = typename std::conditional<
    has_star,
    star,
    typename std::conditional<
      has_plus,
      plus,
      typename std::conditional<
        has_question,
        question,
        typename std::conditional<
          has_curly,
          curly,
          Atom
        >::type
      >::type
    >::type
  >::type;
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

  using chosen = typename std::conditional<
    (Pos >= Pattern.length) || (Pattern[Pos] == '|') || (Pattern[Pos] == ')'),
    impl_empty,
    impl_nonempty
  >::type;
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
  using chosen = typename std::conditional<has_bar, impl_bar, impl_no_bar>::type;
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

namespace onre {
namespace impl {

/* for fast O(|s|) bool matching with little O(1) */
namespace dfa {

/* === classic brzozowski derivative === */
template<typename R, uint8_t C>
struct Derivative;
/* d0/dc = 0 */
template<uint8_t C>
struct Derivative<EmptySet, C> {
  using type = EmptySet;
};
/* de/dc = 0 */
template<uint8_t C>
struct Derivative<Epsilon, C> {
  using type = EmptySet;
};
/* dx/dc = x == c ? e : 0 */
template<uint8_t X, uint8_t C>
struct Derivative<Char<X>, C> {
  using type = typename std::conditional<X == C, Epsilon, EmptySet>::type;
};
template<uint8_t C>
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
    using type = Dispatch;
  #else
    using type = typename std::conditional<C == '\n' || C == '\r', EmptySet, Dispatch>::type;
  #endif
};
template<uint8_t C>
struct Derivative<Wildcard3, C> {
  using type = typename std::conditional<0x80 <= C && C <= 0xBF, Wildcard2, EmptySet>::type;
};
template<uint8_t C>
struct Derivative<Wildcard2, C> {
  using type = typename std::conditional<0x80 <= C && C <= 0xBF, Wildcard1, EmptySet>::type;
};
template<uint8_t C>
struct Derivative<Wildcard1, C> {
  using type = typename std::conditional<0x80 <= C && C <= 0xBF, Epsilon, EmptySet>::type;
};
template <typename CharList, uint8_t C>
struct Derivative<Except<CharList>, C> {
  using type = typename std::conditional<
    0x00 <= C && C <= 0x7F,
    typename std::conditional<
      CharList::template Contains<Char<C>>,
      EmptySet,
      Epsilon
    >::type,
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
};
/* d(R|S)/dc = dR/dc | dS/dc */
template<typename R, typename S, uint8_t C>
struct Derivative<Or<R, S>, C> {
  using type = typename Simplify<
    Or<typename Derivative<R, C>::type, typename Derivative<S, C>::type>
  >::type;
};
/* d(RS)/dc = dR/dc S | (delta(R) ? dS/dc : 0)*/
template<typename L, typename R, uint8_t C>
struct Derivative<Concat<L, R>, C> {
  using Part1 = Concat<typename Derivative<L, C>::type, R>;
  using Part2 = typename std::conditional<
    Nullable<L>::value,
    typename Derivative<R, C>::type,
    EmptySet
  >::type;
  using type = typename Simplify<Or<Part1, Part2>>::type;
};
/* d(R*)/dc = dR/dc R* */
template<typename R, uint8_t C>
struct Derivative<Closure<R>, C> {
    using type = typename Simplify<Concat<typename Derivative<R, C>::type, Closure<R>>>::type;
};

/* === DFA builder === */
template<typename R>
struct RemoveAllAction {
  using type = R;
};
template<size_t I>
struct RemoveAllAction<SetSlot<I>> {
  using type = Epsilon;
};
template<typename L, typename R>
struct RemoveAllAction<Or<L, R>> {
  using type = typename Simplify<
    Or<typename RemoveAllAction<L>::type, typename RemoveAllAction<R>::type>
  >::type;
};
template<typename L, typename R>
struct RemoveAllAction<Concat<L, R>> {
  using type = typename Simplify<
    Concat<typename RemoveAllAction<L>::type, typename RemoveAllAction<R>::type>
  >::type;
};
template<typename R>
struct RemoveAllAction<Closure<R>> {
  using type = typename Simplify<
    Closure<typename RemoveAllAction<R>::type>
  >::type;
};

template<typename R>
struct State {
  using re = R;
  static constexpr bool accepting = Nullable<R>::value;
};

template<std::size_t From, uint8_t C, std::size_t To>
struct Edge {
  static constexpr std::size_t from = From;
  static constexpr uint8_t ch = C;
  static constexpr std::size_t to = To;
};

template<uint8_t C, typename State>
struct CharStatePair {
  static constexpr uint8_t c = C;
  using state = State;
};

template<typename CharStatePairAcc, typename State, typename Alphabet>
struct DerivNewStates;
template<typename Acc, typename S>
struct DerivNewStates<Acc, S, TypeList<>> {
  using type = Acc;
};
template<typename Acc, typename S, uint8_t C, typename... Tails>
struct DerivNewStates<Acc, S, TypeList<Char<C>, Tails...>> {
  using Der = typename Derivative<typename S::re, C>::type;
  using type = typename std::conditional<
    std::is_same_v<Der, EmptySet>,
    std::type_identity<Acc>,
    DerivNewStates<
      typename PushBack<Acc, CharStatePair<C, State<Der>>>::type,
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
  typename HeadPair,
  typename... TailPairs
>
struct PushNewStates<SA, EA, TBP, StartState, TypeList<HeadPair, TailPairs...>> {
  using FromState = StartState;
  using ToState = typename HeadPair::state;
  static constexpr uint8_t C = HeadPair::c;
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
      NextStateAcc::template IndexOf<ToState>
    >
  >::type;
  using NextIt = PushNewStates<
    NextStateAcc,
    NextEdgeAcc,
    NextToBeProcessList,
    StartState,
    TypeList<TailPairs...>
  >;
  using StateAcc = typename NextIt::StateAcc;
  using EdgeAcc = typename NextIt::EdgeAcc;
  using ToBeProcessList = typename NextIt::ToBeProcessList;
};

template<typename StateAcc, typename EdgeAcc, typename ToBeProcessList>
struct BuildDFA;
template<typename StateAcc, typename EdgeAcc>
struct BuildDFA<StateAcc, EdgeAcc, TypeList<>> {
  using States = StateAcc;
  using Edges = EdgeAcc;
};
template<typename StateAcc, typename EdgeAcc, typename StateHead, typename... StateTails>
struct BuildDFA<StateAcc, EdgeAcc, TypeList<StateHead, StateTails...>> {
  using NewCharStates = typename DerivNewStates<
    TypeList<>,
    StateHead,
    typename First<typename StateHead::re, TypeList<>>::type
  >::type;
  using Processed = PushNewStates<
    StateAcc,
    EdgeAcc,
    TypeList<StateTails...>,
    StateHead,
    NewCharStates
  >;
  using NextIt = BuildDFA<
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
  static constexpr std::array<std::array<int32_t, nr_byte>, NrStates> make() {
    std::array<std::array<int32_t, nr_byte>, NrStates> table{};
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
} /* namespace impl */
} /* namespace onre */

namespace onre {

/* === interface === */
template<impl::FixedString Pattern>
inline bool match(std::string_view str) noexcept;
template<impl::FixedString Pattern>
inline std::string replace(std::string_view rule, std::string_view str) noexcept;

template<impl::FixedString Pattern>
inline bool match(std::string_view str) noexcept {
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
inline std::string replace(std::string_view replace_rule, std::string_view str) noexcept {
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

  static auto open_time = [](const SlotLine& line, size_t group_idx) {
    return line[group_idx << 1];
  };
  static auto close_time = [](const SlotLine& line, size_t group_idx) {
    return line[group_idx << 1 | 1];
  };
  static auto is_opened = [](const SlotLine& line, size_t group_idx) {
    return open_time(line, group_idx) >= 0;
  };
  static auto is_closed = [](const SlotLine& line, size_t group_idx) {
    return close_time(line, group_idx) >= 0;
  };
  static auto group_len = [](const SlotLine& line, size_t group_idx) {
    return close_time(line, group_idx) - open_time(line, group_idx);
  };

  static auto is_digit = [](uint8_t ch) { return '0' <= ch && ch <= '9'; };

  static auto apply_action = []<size_t N>(
    const SlotLine& old_line,
    const std::array<int32_t, N>& actions,
    int32_t p
  ) {
    SlotLine new_line{old_line};
    for (const auto& action : actions) {
      if (action < 0) break;
      new_line[action] = p;
    }
    return new_line;
  };

  // heuristically choose a slot configuration to try to get longest match
  static auto need_change = [](const SlotLine& old_line, const SlotLine& new_line) {
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
  };

  thread_local static SlotFile slot_file1, slot_file2;
  thread_local static std::array<bool, nr_states> is_state_active1, is_state_active2;
  thread_local static std::vector<size_t> active_states1, active_states2;

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
          (*nxt_slot_file)[nxt_state] = apply_action(
            (*cur_slot_file)[state],
            trans_action_table[state][static_cast<size_t>(uch)][nxt_state],
            static_cast<int32_t>(idx)
          );
          nxt_active_states->push_back(nxt_state);
          (*nxt_is_state_active)[nxt_state] = true;
          continue;
        }

        auto next_slot_line = apply_action(
          (*cur_slot_file)[state],
          trans_action_table[state][static_cast<size_t>(uch)][nxt_state],
          static_cast<int32_t>(idx)
        );
        if (need_change((*nxt_slot_file)[nxt_state], next_slot_line)) {
          (*nxt_slot_file)[nxt_state] = next_slot_line;
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
      final_line = apply_action((*cur_slot_file)[state], accept_action_table[state], str.size());
      is_final_line_inited = true;
      continue;
    }
    auto after_accept = apply_action((*cur_slot_file)[state], accept_action_table[state], str.size());
    if (need_change(final_line, after_accept)) final_line = after_accept;
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
    else if (is_digit(replace_rule[idx])) {
      size_t group_idx = replace_rule[idx] - '0';
      while (idx + 1 < replace_rule.size() && is_digit(replace_rule[idx + 1])) {
        idx++;
        group_idx = 10 * group_idx + replace_rule[idx] - '0';
      }
      if (group_idx >= nr_capture_group) return "";
      int32_t l = open_time(final_line, group_idx), r = close_time(final_line, group_idx);
      if (l < 0 || r < 0) continue;

      if (r < l) continue;
      result.append(str.data() + l, static_cast<size_t>(r - l));
    }
    else return "";
  }

  return result;
}

} /* namespace onre */ 


namespace onre {
namespace debug {

template <typename RE>
struct DumpRE { static std::string to_string(); };

template <typename TypeList, size_t idx = 0>
struct DumpDFAStates { static std::string to_string(); };

template <typename TypeList, size_t idx = 0>
struct DumpDFAEdges { static std::string to_string(); };

template <typename TypeList, size_t idx = 0>
struct DumpTNFAStates { static std::string to_string(); };

template <typename TypeList, size_t idx = 0>
struct DumpTNFAEdges { static std::string to_string(); };

template <impl::FixedString Pattern>
struct DumpDFA {
  static std::string to_string() {
    using Re = typename impl::RegexScan<Pattern>::type;
    using DFAStates = impl::dfa::AllStatesList<typename impl::dfa::RemoveAllAction<Re>::type>;
    using DFAEdges  = impl::dfa::AllEdgesList<typename impl::dfa::RemoveAllAction<Re>::type>;
    return "States:\n" + DumpDFAStates<DFAStates>::to_string()
      + "Edges:\n" + DumpDFAEdges<DFAEdges>::to_string();
  }
};

template <impl::FixedString Pattern>
struct DumpTNFA {
  static std::string to_string() {
    using Re = typename impl::RegexScan<Pattern>::type;
    using TNFAStates = impl::tnfa::AllStatesList<Re>;
    using TNFAEdges  = impl::tnfa::AllEdgesList<Re>;
    return "States:\n" + DumpTNFAStates<TNFAStates>::to_string()
      + "Edges:\n" + DumpTNFAEdges<TNFAEdges>::to_string();
  }
};

[[maybe_unused]] static std::string char_rep(uint8_t x) {
  if (x >= '!' && x <= '~') {
    static char buf[2];
    buf[0] = x; buf[1] = '\0';
    return std::string(buf);
  }
  else {
    static char buf[5];
    snprintf(buf, sizeof(buf), "\\x%02X", x);
    return std::string(buf, 4);
  }
}

template <typename RE>
struct REPriority : std::integral_constant<int, 0> {};
template <typename R>
struct REPriority<impl::Closure<R>> : std::integral_constant<int, 1> {};
template <typename R, typename S>
struct REPriority<impl::Concat<R, S>> : std::integral_constant<int, 2> {};

template <typename R, typename S>
struct REPriority<impl::Or<R, S>> : std::integral_constant<int, 3> {};

template <typename RE, typename Parent>
struct DumpREWithParen {
  static std::string to_string() {
    if constexpr (REPriority<RE>::value > REPriority<Parent>::value)
      return "(" + DumpRE<RE>::to_string() + ")";
    else
      return DumpRE<RE>::to_string();
  }

};

template <>
struct DumpRE<impl::EmptySet> {
  static std::string to_string() {
    return "(/)";
  }
};

template <>
struct DumpRE<impl::Epsilon> {
  static std::string to_string() {
    return "";
  }
};

template <uint8_t C>
struct DumpRE<impl::Char<C>> {
  static std::string to_string() {
    return char_rep(C);
  }
};

template <size_t I>
struct DumpRE<impl::SetSlot<I>> {
  static std::string to_string() {
    return "<" + std::to_string(I) + ">";
  }
};

template<>
struct DumpRE<impl::Wildcard> {
  static std::string to_string() {
    return std::string(".");
  }
};
template<>
struct DumpRE<impl::Wildcard3> {
  static std::string to_string() {
    return std::string(".(3)");
  }
};
template<>
struct DumpRE<impl::Wildcard2> {
  static std::string to_string() {
    return std::string(".(2)");
  }
};
template<>
struct DumpRE<impl::Wildcard1> {
  static std::string to_string() {
    return std::string(".(1)");
  }
};

template<typename CharList>
struct CharListToString;
template<uint8_t C, typename... Tail>
struct CharListToString<impl::TypeList<impl::Char<C>, Tail...>> {
  static std::string to_string() {
    return char_rep(C) + CharListToString<impl::TypeList<Tail...>>::to_string();
  }
};
template<>
struct CharListToString<impl::TypeList<>> {
  static std::string to_string() {
    return "";
  }
};

template<typename List>
struct DumpRE<impl::Except<List>> {
  static std::string to_string() {
    return std::string("Except<")
      + CharListToString<List>::to_string()
      + ">";
  }
};

template <typename R, typename S>
struct DumpRE<impl::Or<R, S>> {
  static std::string to_string() {
    return DumpREWithParen<R, impl::Or<R, S>>::to_string()
      + '|' + DumpREWithParen<S, impl::Or<R, S>>::to_string();
  }
};

template <typename R, typename S>
struct DumpRE<impl::Concat<R, S>> {
  template <typename T> struct is_concat : std::false_type {};
  template <typename U, typename T> struct is_concat<impl::Concat<U, T>> : std::true_type {};
  static std::string to_string() {
    return DumpREWithParen<R, impl::Concat<R, S>>::to_string()
      + DumpREWithParen<S, impl::Concat<R, S>>::to_string();
  }
};

template <typename R>
struct DumpRE<impl::Closure<R>> {
  static std::string to_string() {
    return DumpREWithParen<R, impl::Closure<R>>::to_string() + '*';
  }
};

template<>
struct DumpRE<impl::Omega> {
  static std::string to_string() {
    return "<o>";
  }
};

template<size_t I>
struct DumpRE<impl::Set<I>> {
  static std::string to_string() {
    return "<" + std::to_string(I) +">";
  }
};

template<typename Head, typename... Tails>
struct DumpRE<impl::Seq<Head, Tails...>> {
  static std::string to_string() {
    return DumpRE<Head>::to_string() + DumpRE<impl::Seq<Tails...>>::to_string();
  }
};

template<>
struct DumpRE<impl::Seq<>> {
  static std::string to_string() {
    return "";
  }
};

template <typename RE, typename... Tails, size_t idx>
struct DumpDFAStates<impl::TypeList<impl::dfa::State<RE>, Tails...>, idx> {
  static std::string to_string() {
    return "(" + std::to_string(idx) + ") " +
      (impl::dfa::State<RE>::accepting ? "(*) " : "    ")
      + DumpRE<RE>::to_string() + "\n"
      + DumpDFAStates<impl::TypeList<Tails...>, idx + 1>::to_string();
  }
};

template <size_t idx>
struct DumpDFAStates<impl::TypeList<>, idx> {
  static std::string to_string() { return ""; }
};

template<typename List> struct ActionTypeListToString;
template<typename Head, typename... Tails> struct ActionTypeListToString<impl::TypeList<Head, Tails...>> {
  static std::string to_string() {
    return DumpRE<Head>::to_string() + (sizeof...(Tails) > 0 ? ", " : "") + ActionTypeListToString<impl::TypeList<Tails...>>::to_string();
  }
};
template<> struct ActionTypeListToString<impl::TypeList<>> {
  static std::string to_string() { return ""; }
};


template<size_t idx, typename RE, typename... Tails>
struct DumpTNFAStates<impl::TypeList<impl::tnfa::State<RE>, Tails...>, idx> {
  static std::string to_string() {
    return "(" + std::to_string(idx) + ") "
      + (impl::tnfa::State<RE>::accepting ? "(*) " : "    ")
      + DumpRE<typename impl::dfa::RemoveAllAction<RE>::type>::to_string()
      + " : " + DumpRE<RE>::to_string()
      + " : {" + ActionTypeListToString<typename impl::tnfa::v<RE>::type>::to_string() + "}\n"
      + DumpTNFAStates<impl::TypeList<Tails...>, idx + 1>::to_string();
  }
};

template <size_t idx>
struct DumpTNFAStates<impl::TypeList<>, idx> {
  static std::string to_string() { return ""; }
};

template<size_t idx, size_t From, uint8_t C, size_t To, typename... Tails>
struct DumpDFAEdges<impl::TypeList<impl::dfa::Edge<From, C, To>, Tails...>, idx> {
  static std::string to_string() {
    return "(" + std::to_string(idx) + ") "
      + std::to_string(From) + " --" + char_rep(C) + "--> " + std::to_string(To) + "\n"
      + DumpDFAEdges<impl::TypeList<Tails...>, idx + 1>::to_string();
  }
};

template <size_t idx>
struct DumpDFAEdges<impl::TypeList<>, idx> {
  static std::string to_string() { return ""; }
};

template<size_t idx, size_t From, uint8_t C, typename Action, size_t To, typename... Tails>
struct DumpTNFAEdges<impl::TypeList<impl::tnfa::Edge<From, C, Action, To>, Tails...>, idx> {
  static std::string to_string() {
    return "(" + std::to_string(idx) + ") "
      + std::to_string(From) + " --" + char_rep(C)
      + "[" + DumpRE<Action>::to_string() + "]--> " + std::to_string(To) + "\n"
      + DumpTNFAEdges<impl::TypeList<Tails...>, idx + 1>::to_string();
  }
};

template <size_t idx>
struct DumpTNFAEdges<impl::TypeList<>, idx> {
  static std::string to_string() { return ""; }
};

} /* namespace debug */
} /* namespace onre */


#endif /* #ifndef ONRE_REGEX_HPP__ */
