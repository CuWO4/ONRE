#ifndef ONRE_DEBUG_HXX__
#define ONRE_DEBUG_HXX__

#include "head.hxx"
#include "fixedstring.hxx"
#include "regexscan.hxx"
#include "dfa.hxx"
#include "tnfa.hxx"

// === snippet begin ===

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

// === snippet end ===

#endif /* !ONRE_DEBUG_HXX__ */
