#ifndef REG_HXX__
#define REG_HXX__

#include "head.hxx"
#include "alphabet.hxx"

// === snippet begin ===
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
template<typename List1, typename List2, typename List3, typename List4>
struct In {};
template<typename List2, typename List3, typename List4>
struct In3 {};
template<typename List3, typename List4>
struct In2 {};
template<typename List4>
struct In1 {};
template<typename List1, typename List2, typename List3, typename List4>
struct Except {};
template<typename List2, typename List3, typename List4>
struct Except3 {};
template<typename List3, typename List4>
struct Except2 {};
template<typename List4>
struct Except1 {};
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
struct Nullable {};
template<>
struct Nullable<EmptySet> : std::false_type {};
template<>
struct Nullable<Epsilon> : std::true_type {};
template<uint8_t C>
struct Nullable<Char<C>> : std::false_type {};
template<>
struct Nullable<Wildcard> : std::false_type {};
template<>
struct Nullable<Wildcard3> : std::false_type {};
template<>
struct Nullable<Wildcard2> : std::false_type {};
template<>
struct Nullable<Wildcard1> : std::false_type {};
template<typename... Lists>
struct Nullable<In<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<In3<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<In2<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<In1<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<Except<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<Except3<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<Except2<Lists...>> : std::false_type {};
template<typename... Lists>
struct Nullable<Except1<Lists...>> : std::false_type {};
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
template<typename List1, typename... Lists, typename Acc>
struct First<In<List1, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, List1>::type;
};
template<typename List2, typename... Lists, typename Acc>
struct First<In3<List2, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, List2>::type;
};
template<typename List3, typename... Lists, typename Acc>
struct First<In2<List3, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, List3>::type;
};
template<typename List4, typename Acc>
struct First<In1<List4>, Acc> {
  using type = typename JoinUnique<Acc, List4>::type;
};
template<typename List1, typename... Lists, typename Acc>
struct First<Except<List1, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, typename CharListNegation<List1, Utf8FirstByteAlphabet>::type>::type;
};
template<typename List2, typename... Lists, typename Acc>
struct First<Except3<List2, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, typename CharListNegation<List2, Utf8ContinuationByteAlphabet>::type>::type;
};
template<typename List3, typename... Lists, typename Acc>
struct First<Except2<List3, Lists...>, Acc> {
  using type = typename JoinUnique<Acc, typename CharListNegation<List3, Utf8ContinuationByteAlphabet>::type>::type;
};
template<typename List4, typename Acc>
struct First<Except1<List4>, Acc> {
  using type = typename JoinUnique<Acc, typename CharListNegation<List4, Utf8ContinuationByteAlphabet>::type>::type;
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
  using type = typename std::conditional_t<
    Nullable<R>::value,
    impl_r_nullable,
    impl_r_non_nullable
  >::type;
};
template <typename R, typename Acc>
struct First<Closure<R>, Acc> {
  using type = typename First<R, Acc>::type;
};

} /* namespace impl */
} /* namespace onre */

// === snippet end ===

#endif