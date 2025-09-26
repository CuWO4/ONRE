#include <array>
#include <cstddef>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include <cstdio>

namespace onre {

namespace impl {

struct RE {};
struct EmptySet : RE {};
struct Epsilon : RE {};
template<char C> struct Char : RE { static constexpr char c = C; };
template<typename R, typename S> struct Or : RE { using left = R; using right = S; };
template<typename R, typename S> struct Concat : RE {using left = R; using right = S; };
template<typename R> struct Closure : RE { using inner = R; };

template<typename R>
struct Nullable {};
/* delta(0) = F */
template<> struct Nullable<EmptySet> : std::false_type {};
/* delta(e) = T */
template<> struct Nullable<Epsilon> : std::true_type {};
/* delta(c) = T */
template<char C> struct Nullable<Char<C>> : std::false_type {};
/* delta(R|S) = delta(R) || delta(S) */
template<typename L, typename R> struct Nullable<Or<L, R>> : std::bool_constant<Nullable<L>::value || Nullable<R>::value> {};
/* delta(RS) = delta(R) && delta(S) */
template<typename L, typename R> struct Nullable<Concat<L, R>> : std::bool_constant<Nullable<L>::value && Nullable<R>::value> {};
/* delta(R*) = T */
template<typename R> struct Nullable<Closure<R>> : std::true_type {};

template<typename R> struct Simplify { using type = R; };

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

/* e|R <=> R|e <=> R */
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

/* R** <=> R* */
template<typename R> struct Simplify<Closure<Closure<R>>> { using type = typename Simplify<Closure<R>>::type; };

/* (e|R)* <=> (R|e)* <=> R* */
template<typename R> struct Simplify<Closure<Or<Epsilon, R>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Closure<Or<R, Epsilon>>> { using type = typename Simplify<Closure<R>>::type; };
template<> struct Simplify<Closure<Or<Epsilon, Epsilon>>> { using type = Epsilon; };
/* e|RR* <=> RR*|e <=> R* */
template<typename R> struct Simplify<Or<Epsilon, Concat<R, Closure<R>>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Or<Concat<R, Closure<R>>, Epsilon>> { using type = typename Simplify<Closure<R>>::type; };

/* standard ordering */
template<typename A, typename B>
struct is_less : std::false_type {};

/* Or < Concat < Closure < EmptySet < Epsilon < Char */
template <typename R1, typename S1, typename R2, typename S2> struct is_less<Or<R1, S1>, Concat<R2, S2>> : std::true_type {};
template <typename R1, typename S1, typename R2> struct is_less<Or<R1, S1>, Closure<R2>> : std::true_type {};
template <typename R, typename S> struct is_less<Or<R, S>, EmptySet> : std::true_type {};
template <typename R, typename S> struct is_less<Or<R, S>, Epsilon> : std::true_type {};
template <typename R, typename S, char C> struct is_less<Or<R, S>, Char<C>> : std::true_type {};
template <typename R1, typename S1, typename R2> struct is_less<Concat<R1, S1>, Closure<R2>> : std::true_type {};
template <typename R, typename S> struct is_less<Concat<R, S>, EmptySet> : std::true_type {};
template <typename R, typename S> struct is_less<Concat<R, S>, Epsilon> : std::true_type {};
template <typename R, typename S, char C> struct is_less<Concat<R, S>, Char<C>> : std::true_type {};
template <typename R> struct is_less<Closure<R>, EmptySet> : std::true_type {};
template <typename R> struct is_less<Closure<R>, Epsilon> : std::true_type {};
template <typename R, char C> struct is_less<Closure<R>, Char<C>> : std::true_type {};
template <> struct is_less<EmptySet, Epsilon> : std::true_type {};
template <char C> struct is_less<EmptySet, Char<C>> : std::true_type {};
template <char C> struct is_less<Epsilon, Char<C>> : std::true_type {};

template <char C1, char C2>
struct is_less<Char<C1>, Char<C2>> { static constexpr bool value = C1 < C2; };
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

/*
  Grammar:
    Regex       := Term ('|' Regex)?
    Term        := Factor Term | (empty)
    Factor      := Atom ('*')? | Atom ('+')? | Atom ('?')?
    Atom        := '(' Regex ')' | '[' CharSet ']' | CHAR
    CharSet     := CharSetAtom CharSet | CharSetAtom
    CharSetAtom := CHAR | CHAR '-' CHAR
    CHAR        := [0-9a-zA-Z]
    Empty input -> Epsilon
*/

/* forward declarations */
template<FixedString Pattern, size_t Pos> struct ParseRegex;
template<FixedString Pattern, size_t Pos> struct ParseTerm;
template<FixedString Pattern, size_t Pos> struct ParseFactor;
template<FixedString Pattern, size_t Pos> struct ParseAtom;
template<FixedString Pattern, size_t Pos> struct ParseCharSet;
template<FixedString Pattern, size_t Pos> struct ParseCharSetAtom;

constexpr bool is_valid_char(char c) {
  return (c >= '0' && c <= '9')
    || (c >= 'a' && c <= 'z')
    || (c >= 'A' && c <= 'Z');
}

template<char Start, char End>
struct CharOrSequential {
  static_assert(Start <= End, "CharOrSequential: invalid range");
  using type = Or<Char<Start>, typename CharOrSequential<Start + 1, End>::type>;
};

template<char C>
struct CharOrSequential<C, C> {
  using type = Char<C>;
};

/* ParseCharSetAtom := CHAR | CHAR '-' CHAR */
template<FixedString Pattern, size_t Pos>
struct ParseCharSetAtom {
  static_assert(Pos < Pattern.length, "ParseCharSetAtom: unexpected pattern ending");
  static_assert(is_valid_char(Pattern[Pos]), "ParseCharSetAtom: unknown character");

  struct impl_seq {
    static_assert(Pos + 2 < Pattern.length && is_valid_char(Pattern[Pos + 2]), "ParseCharSetAtom: `-` has no ending");
    using type = typename CharOrSequential<Pattern[Pos], Pattern[Pos + 2]>::type;
    static constexpr size_t next = Pos + 3;
  };

  struct impl_char {
    using type = Char<Pattern[Pos]>;
    static constexpr size_t next = Pos + 1;
  };

  static constexpr bool has_hyphen = Pos + 1 < Pattern.length && Pattern[Pos + 1] == '-';
  using chosen = std::conditional_t<has_hyphen, impl_seq, impl_char>;
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
    using type = Or<typename CharSetAtom::type, typename Next::type>;
    static constexpr size_t next = Next::next;
  };

  struct impl_stop {
    using type = typename CharSetAtom::type;
    static constexpr size_t next = CharSetAtom::next;
  };

  static constexpr bool run_on = CharSetAtom::next < Pattern.length && is_valid_char(Pattern[CharSetAtom::next]);
  using chosen = std::conditional_t<run_on, impl_run_on, impl_stop>;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

/* ParseAtom: '(' Regex ')' | '[' CharSet ']' |  CHAR */
template<FixedString Pattern, size_t Pos>
struct ParseAtom {
  static_assert(Pos < Pattern.length && (Pattern[Pos] == '(' || Pattern[Pos] == '[' || is_valid_char(Pattern[Pos])),
    "ParseAtom: unknown character");

  /* case '(' Regex ')' */
  struct impl_paren {
    using Regex = ParseRegex<Pattern, Pos + 1>;
    static_assert(Regex::next <= Pattern.length, "ParseAtom: regex parse overflow");
    static_assert(Regex::next < Pattern.length && Pattern[Regex::next] == ')',
      "ParseAtom impl_paren: missing closing ')' in pattern");
    using type = typename Regex::type;
    static constexpr size_t next = Regex::next + 1;
  };

  /* case '[' CharSet ']'*/
  struct impl_square {
    using CharSet = ParseCharSet<Pattern, Pos + 1>;
    static_assert(CharSet::next <= Pattern.length, "ParseAtom: char set parse overflow");
    static_assert(CharSet::next < Pattern.length && Pattern[CharSet::next] == ']',
      "ParseAtom impl_paren: missing closing ']' in pattern");
    using type = typename CharSet::type;
    static constexpr size_t next = CharSet::next + 1;
  };

  /* case CHAR */
  struct impl_char {
    using type = Char<Pattern[Pos]>;
    static constexpr size_t next = Pos + 1;
  };

  using chosen = std::conditional_t<
    Pattern[Pos] == '(',
    impl_paren,
    std::conditional_t<
      Pattern[Pos] == '[',
      impl_square,
      impl_char
    >
  >;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

/* ParseFactor: Atom ('*')? | Atom ('+')? | Atom ('?')? */
template<FixedString Pattern, size_t Pos>
struct ParseFactor {
  using Atom = ParseAtom<Pattern, Pos>;

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
};

/* ParseTerm := Factor Term | (empty -> Epsilon) */
template<FixedString Pattern, size_t Pos>
struct ParseTerm {
  /* empty case */
  struct impl_empty {
    using type = Epsilon;
    static constexpr size_t next = Pos;
  };
  /* Factor Term case */
  struct impl_nonempty {
    using Factor = ParseFactor<Pattern, Pos>;
    static_assert(Factor::next <= Pattern.length, "ParseTerm: factor parse overflow");
    using Term = ParseTerm<Pattern, Factor::next>;
    static_assert(Term::next <= Pattern.length, "ParseTerm: term parse overflow");
    using type = Concat<typename Factor::type, typename Term::type>;
    static constexpr size_t next = Term::next;
  };

  using chosen = std::conditional_t<
    (Pos >= Pattern.length) || (Pattern[Pos] == '|') || (Pattern[Pos] == ')'),
    impl_empty,
    impl_nonempty
  >;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

/* ParseRegex := Term ('|' Regex)? */
template<FixedString Pattern, size_t Pos>
struct ParseRegex {
  using Term = ParseTerm<Pattern, Pos>;

  static_assert(Term::next <= Pattern.length, "ParseRegex: term parse overflow");

  struct impl_bar {
    using Regex = ParseRegex<Pattern, Term::next + 1>;
    static_assert(Regex::next <= Pattern.length, "ParseRegex: regex parse overflow");
    using type = Or<typename Term::type, typename Regex::type>;
    static constexpr size_t next = Regex::next;
  };
  struct impl_no_bar {
    using type = typename Term::type;
    static constexpr size_t next = Term::next;
  };

  static constexpr bool has_bar = (Term::next < Pattern.length && Pattern[Term::next] == '|');
  using chosen = std::conditional_t<has_bar, impl_bar, impl_no_bar>;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

template<FixedString Pattern>
struct RegexScan {
  using Parse = ParseRegex<Pattern, 0>;
  using type = typename Parse::type;
  static_assert(Parse::next == Pattern.length, "RegexScan: pattern not fully consumed or contains unexpected trailing characters");
};

template<typename... Ts> struct TypeList {};
template<typename List, typename T> struct Contains;
template<typename T, typename... Ts> struct Contains<TypeList<Ts...>, T> : std::disjunction<std::is_same<T, Ts>...> {};
template<typename List, typename T>
struct PushBackUnique;
template<typename... Ts, typename T>
struct PushBackUnique<TypeList<Ts...>, T> {
  using type = std::conditional_t<
    Contains<TypeList<Ts...>, T>::value,
    TypeList<Ts...>,
    TypeList<Ts..., T>
  >;
};

template<typename R>
struct State {
  using re = R;
  static constexpr bool accepting = Nullable<R>::value;
};

template<typename List> struct TypeListLength;
template<typename... Ts> struct TypeListLength<TypeList<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

using Alphabet = TypeList<
  Char<'0'>, Char<'1'>, Char<'2'>, Char<'3'>, Char<'4'>, Char<'5'>, Char<'6'>, Char<'7'>, Char<'8'>, Char<'9'>,
  Char<'a'>, Char<'b'>, Char<'c'>, Char<'d'>, Char<'e'>, Char<'f'>, Char<'g'>, Char<'h'>, Char<'i'>, Char<'j'>,
  Char<'k'>, Char<'l'>, Char<'m'>, Char<'n'>, Char<'o'>, Char<'p'>, Char<'q'>, Char<'r'>, Char<'s'>, Char<'t'>,
  Char<'u'>, Char<'v'>, Char<'w'>, Char<'x'>, Char<'y'>, Char<'z'>,
  Char<'A'>, Char<'B'>, Char<'C'>, Char<'D'>, Char<'E'>, Char<'F'>, Char<'G'>, Char<'H'>, Char<'I'>, Char<'J'>,
  Char<'K'>, Char<'L'>, Char<'M'>, Char<'N'>, Char<'O'>, Char<'P'>, Char<'Q'>, Char<'R'>, Char<'S'>, Char<'T'>,
  Char<'U'>, Char<'V'>, Char<'W'>, Char<'X'>, Char<'Y'>, Char<'Z'>
>;

template<std::size_t From, char C, std::size_t To>
struct Edge { static constexpr std::size_t from = From; static constexpr char ch = C; static constexpr std::size_t to = To; };

template<typename StatesList, typename EdgesList>
struct StateEdgePair {
  using States = StatesList;
  using Edges = EdgesList;
};

/* find index of T in TypeList<Ls...> */
template<typename List, typename T, std::size_t I = 0> struct IndexOf;
template<typename Head, typename... Tail, typename T, std::size_t I>
struct IndexOf<TypeList<Head, Tail...>, T, I> : IndexOf<TypeList<Tail...>, T, I + 1> {};
template<typename Head, typename... Tail, std::size_t I>
struct IndexOf<TypeList<Head, Tail...>, Head, I> : std::integral_constant<std::size_t, I> {};
template<typename T, std::size_t I>
struct IndexOf<TypeList<>, T, I> { static_assert(sizeof(T) == 0, "IndexOf: type not found in TypeList"); };


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
          IndexOf<typename PushBackUnique<current_states, State<Der_t>>::type, StateT>::value,
          C,
          IndexOf<typename PushBackUnique<current_states, State<Der_t>>::type, State<Der_t>>::value
        >
      >::type
    >
  >;
};

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

/* ExpandOnce on a list of states: produce Pair<NewStates, NewEdges> */
template<typename StateEdgePair>
struct ExpandOnce {
private:
  /* fold over each state: for each state, append derivatives across alphabet */
  template<typename PairSoFar, typename StateT>
  struct DoState {
    using type = typename AppendDerivativesForStatePair<PairSoFar, StateT, Alphabet>::type;
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

/* ExpandUntilStable: iterate ExpandOncePair until states length doesn't increase */
template<typename PairAcc, std::size_t PrevLen>
struct ExpandUntilStablePairImpl;

template<typename PairAcc, std::size_t PrevLen>
struct ExpandUntilStablePairImpl {
private:
  using NextPair = typename ExpandOnce<PairAcc>::type;
  static constexpr std::size_t next_len = TypeListLength<typename NextPair::States>::value;

public:
  using type = typename std::conditional_t<
    next_len == PrevLen,
    std::type_identity<NextPair>,
    ExpandUntilStablePairImpl<NextPair, next_len>
  >::type;
};

template<typename PairAcc, std::size_t PrevLen = TypeListLength<typename PairAcc::States>::value>
struct ExpandUntilStablePair {
  using type = typename ExpandUntilStablePairImpl<PairAcc, PrevLen>::type;
};

template<typename RE>
struct AllStatesAndEdgesGenerator {
private:
  using initial_states = TypeList<State<RE>>;
  using initial_pair = StateEdgePair<initial_states, TypeList<>>;
public:
  using type = typename ExpandUntilStablePair<initial_pair, TypeListLength<initial_states>::value>::type;
  using States = typename type::States;
  using Edges  = typename type::Edges;
};

/* aliases */
template<typename RE> using AllStateEdgePair = typename AllStatesAndEdgesGenerator<RE>::type;
template<typename RE> using AllStatesList = typename AllStatesAndEdgesGenerator<RE>::States;
template<typename RE> using AllEdgesList  = typename AllStatesAndEdgesGenerator<RE>::Edges;

static constexpr size_t NR_ASCII_CHAR = 128;

template<typename EdgesList>
struct BuildTable {
  template<std::size_t N>
  static consteval std::array<std::array<int, NR_ASCII_CHAR>, N> make() {
    std::array<std::array<int, NR_ASCII_CHAR>, N> table{};
    for (auto &row : table) row.fill(-1);
    return table;
  }
};
template<typename... Edges>
struct BuildTable<TypeList<Edges...>> {
  template<std::size_t N>
  static consteval std::array<std::array<int, NR_ASCII_CHAR>, N> make() {
    std::array<std::array<int, NR_ASCII_CHAR>, N> table{};
    for (auto &row : table) row.fill(-1);
    ((table[Edges::from][Edges::ch] = Edges::to), ...);
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

} /* namespace impl */

template<impl::FixedString Pattern>
class Regex {
public:
  static bool match(std::string_view str) {
    std::size_t cur = 0;
    for (char ch : str) {
      int nxt = trans[cur][static_cast<std::size_t>(ch)];
      if (nxt < 0) return false;
      cur = static_cast<std::size_t>(nxt);
    }
    return accepts[cur];
  }

private:
  using Re = typename impl::RegexScan<Pattern>::type;
  using Pair = impl::AllStateEdgePair<Re>;
  using StatesList = typename Pair::States;
  using EdgesList  = typename Pair::Edges;
  static constexpr std::size_t nr_states = impl::TypeListLength<StatesList>::value;
  static constexpr auto trans = impl::BuildTable<EdgesList>::template make<nr_states>();
  static constexpr auto accepts = impl::BuildAccepts<StatesList>::make();
};

namespace logger {

  template <typename RE>
  struct ToString;

  template <>
  struct ToString<impl::EmptySet> {
    static std::string to_string() {
      return "(/)";
    }
  };

  template <>
  struct ToString<impl::Epsilon> {
    static std::string to_string() {
      return "";
    }
  };

  template <char C>
  struct ToString<impl::Char<C>> {
    static std::string to_string() {
      return std::string(1, C);
    }
  };

  template <typename R, typename S>
  struct ToString<impl::Or<R, S>> {
    static std::string to_string() {
      return ToString<R>::to_string() + '|' + ToString<S>::to_string();
    }
  };

  template <typename R, typename S>
  struct ToString<impl::Concat<R, S>> {
    static std::string to_string() {
      auto left_str = impl::is_less<R, impl::Concat<R, S>>::value
        ? '(' + ToString<R>::to_string() + ")" : ToString<R>::to_string();
      auto right_str = impl::is_less<S, impl::Concat<R, S>>::value
        ? '(' + ToString<S>::to_string() + ")" : ToString<S>::to_string();
      return  left_str + right_str;
    }
  };

  template <typename R>
  struct ToString<impl::Closure<R>> {
    static std::string to_string() {
      return impl::is_less<R, impl::Closure<R>>::value
        ? '(' + ToString<R>::to_string() + ")*"
        : ToString<R>::to_string();
    }
  };

  template <typename TypeList, size_t idx>
  struct TypeListPrinterImpl;

  template <typename TypeList>
  struct TypeListPrinter {
    static void print() {
      TypeListPrinterImpl<TypeList, 0>::print();
    }
  };

  template<size_t idx, typename RE, typename... Remains>
  struct TypeListPrinterImpl<impl::TypeList<impl::State<RE>, Remains...>, idx> {
    static void print() {
      printf("(%lu) %s\n", idx, ToString<RE>::to_string().c_str());
      TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
    }
  };

  template<size_t idx, size_t From, char C, size_t To, typename... Remains>
  struct TypeListPrinterImpl<impl::TypeList<impl::Edge<From, C, To>, Remains...>, idx> {
    static void print() {
      printf("(%lu) %lu --%c-> %lu\n", idx, From, C, To);
      TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
    }
  };

  template <size_t idx>
  struct TypeListPrinterImpl<impl::TypeList<>, idx> {
    static void print() {}
  };

} /* namespace logger */

} /* namespace onre */
