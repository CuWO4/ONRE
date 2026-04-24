#ifndef TYPELIST_HXX__
#define TYPELIST_HXX__

#include "head.hxx"

// === snippet begin ===
namespace onre {
namespace impl {

/* === type list, a linear container of types === */
template<typename... Ts>
struct TypeList {
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

template <typename List>
struct Unique;
template <typename Head, typename... Tails>
struct Unique<TypeList<Head, Tails...>> {
  using type = std::conditional_t<
    TypeList<Tails...>::template Contains<Head>, 
    typename Unique<TypeList<Tails...>>::type,
    typename PushFront<typename Unique<TypeList<Tails...>>::type, Head>::type
  >;
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
  using type = typename std::conditional_t<
    IsKeep<Head>::value,
    FilterImpl<IsKeep, TypeList<Tails...>, typename PushBack<Acc, Head>::type>,
    FilterImpl<IsKeep, TypeList<Tails...>, Acc>
  >::type;
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

// === snippet end ===

#endif