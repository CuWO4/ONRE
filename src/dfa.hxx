#ifndef DFA_HXX__
#define DFA_HXX__

#include "reg.hxx"
#include "typelist.hxx"
#include "simplify.hxx"


// === snippet begin ===
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
  using type = std::conditional_t<X == C, Epsilon, EmptySet>;
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
    std::conditional_t<
      CharList::template Contains<Char<C>>,
      EmptySet,
      Epsilon
    >,
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
  using Part2 = std::conditional_t<
    Nullable<L>::value,
    typename Derivative<R, C>::type,
    EmptySet
  >;
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
  using type = typename std::conditional_t<
    std::is_same_v<Der, EmptySet>,
    std::type_identity<Acc>,
    DerivNewStates<
      typename PushBack<Acc, CharStatePair<C, State<Der>>>::type,
      S,
      TypeList<Tails...>
    >
  >::type;
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
  using NextToBeProcessList = typename std::conditional_t<
    IsStateNew,
    PushBack<TBP, ToState>,
    std::type_identity<TBP>
  >::type;
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

// === snippet end ===

#endif