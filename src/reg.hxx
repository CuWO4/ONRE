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