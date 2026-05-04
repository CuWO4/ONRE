#ifndef SIMPLIFY_HXX_xC8ZmzgF_
#define SIMPLIFY_HXX_xC8ZmzgF_

#include "reg.hxx"

// === snippet begin ===
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

// === snippet end ===

#endif