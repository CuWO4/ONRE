#include <array>
#include <cstddef>
#include <type_traits>
#include <string_view>
#include <algorithm>

namespace onre {

struct RE {};
struct EmptySet : RE {};
struct Epsilon : RE {};
template<char C> struct Char : RE {};
template<typename R, typename S> struct Or : RE {};
template<typename R, typename S> struct Concat : RE {};
template<typename R> struct Closure : RE {};

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

/* 0R <=> R0 <=> 0 */
template<typename R> struct Simplify<Concat<EmptySet, R>> { using type = EmptySet; };
template<typename L> struct Simplify<Concat<L, EmptySet>> { using type = EmptySet; };
/* eR <=> Re <=> R */
template<typename R> struct Simplify<Concat<Epsilon, R>> { using type = typename Simplify<R>::type; };
template<typename L> struct Simplify<Concat<L, Epsilon>> { using type = typename Simplify<L>::type; };

/* R** <=> R* */
template<typename R> struct Simplify<Closure<Closure<R>>> { using type = typename Simplify<Closure<R>>::type; };

/* (e|R)* <=> (R|e)* <=> R* */
template<typename R> struct Simplify<Closure<Or<Epsilon, R>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Closure<Or<R, Epsilon>>> { using type = typename Simplify<Closure<R>>::type; };
/* R|SR <=> SR|R <=> (e|S)R */
template<typename R, typename S> struct Simplify<Or<R, Concat<S, R>>> { using type = typename Simplify<Concat<Or<Epsilon, S>, R>>::type; };
template<typename R, typename S> struct Simplify<Or<Concat<S, R>, R>> { using type = typename Simplify<Concat<Or<Epsilon, S>, R>>::type; };
/* e|RR* <=> RR*|e <=> R* */
template<typename R> struct Simplify<Or<Epsilon, Concat<R, Closure<R>>>> { using type = typename Simplify<Closure<R>>::type; };
template<typename R> struct Simplify<Or<Concat<R, Closure<R>>, Epsilon>> { using type = typename Simplify<Closure<R>>::type; };

/* recursive */
template<typename L, typename R> struct Simplify<Or<L, R>> { using type = Or<typename Simplify<L>::type, typename Simplify<R>::type>; };
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
    Regex   := Term ('|' Regex)?
    Term    := Factor Term | (empty)
    Factor  := Atom ('*')?
    Atom    := '(' Regex ')' | CHAR
    CHAR    := [0-9a-zA-Z]
    Empty input -> Epsilon
*/

/* forward declarations */
template<FixedString Pattern, size_t Pos> struct ParseRegex;
template<FixedString Pattern, size_t Pos> struct ParseTerm;
template<FixedString Pattern, size_t Pos> struct ParseFactor;
template<FixedString Pattern, size_t Pos> struct ParseAtom;

constexpr bool is_valid_char(char c) {
  return (c >= '0' && c <= '9')
    || (c >= 'a' && c <= 'z')
    || (c >= 'A' && c <= 'Z');
}

/* ParseAtom: '(' Regex ')'  |  CHAR */
template<FixedString Pattern, size_t Pos>
struct ParseAtom {
  static_assert(Pos < Pattern.length && (Pattern[Pos] == '(' || is_valid_char(Pattern[Pos])), "ParseAtom: Failed to parse");

  /* case '(' Regex ')' */
  struct impl_paren {
    using Regex = ParseRegex<Pattern, Pos + 1>;
    static_assert(Regex::next <= Pattern.length, "ParseAtom: regex parse overflow");
    static_assert(Regex::next < Pattern.length && Pattern[Regex::next] == ')', "ParseAtom impl_paren: missing closing ')' in pattern");
    using type = typename Regex::type;
    static constexpr size_t next = Regex::next + 1;
  };

  /* case CHAR */
  struct impl_char {
    using type = Char<Pattern[Pos]>;
    static constexpr size_t next = Pos + 1;
  };

  using chosen = std::conditional_t<
    Pattern[Pos] == '(',
    impl_paren,
    impl_char
  >;
  using type = typename Simplify<typename chosen::type>::type;
  static constexpr size_t next = chosen::next;
};

/* ParseFactor := Atom ('*')? */
template<FixedString Pattern, size_t Pos>
struct ParseFactor {
  using Atom = ParseAtom<Pattern, Pos>;

  static_assert(Atom::next <= Pattern.length, "ParseFactor: atom parse overflow");

  static constexpr bool has_star = (Atom::next < Pattern.length && Pattern[Atom::next] == '*');
  using type = typename Simplify<std::conditional_t<
    has_star,
    Closure<typename Atom::type>,
    typename Atom::type
  >>::type;
  static constexpr size_t next = has_star ? (Atom::next + 1) : Atom::next;
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

// TODO: AllStatesGenerator<RE> => TypeList<State>

template<FixedString Pattern>
class Regex {
public:
  static bool match(std::string_view str) {
    /* TODO */
  }
};

} /* namespace onre */