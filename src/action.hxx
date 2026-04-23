#ifndef ACTION_HXX__
#define ACTION_HXX__

#include "head.hxx"

// === snippet begin ===
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

// === snippet end ===


#endif