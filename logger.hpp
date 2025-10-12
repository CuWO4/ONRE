#ifndef ONRE_LOGGER_HPP_
#define ONRE_LOGGER_HPP_

#include "regex.hpp"

namespace onre {
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

template <size_t I>
struct ToString<impl::SetSlot<I>> {
  static std::string to_string() {
    return "<" + std::to_string(I) + ">";
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
  template <typename T> struct is_concat : std::false_type {};
  template <typename U, typename T> struct is_concat<impl::Concat<U, T>> : std::true_type {};
  static std::string to_string() {
    auto left_str = !is_concat<R>::value && impl::is_less<R, impl::Concat<R, S>>::value
      ? '(' + ToString<R>::to_string() + ")" : ToString<R>::to_string();
    auto right_str = !is_concat<S>::value && impl::is_less<S, impl::Concat<R, S>>::value
      ? '(' + ToString<S>::to_string() + ")" : ToString<S>::to_string();
    return  left_str + right_str;
  }
};

template <typename R>
struct ToString<impl::Closure<R>> {
  static std::string to_string() {
    return impl::is_less<R, impl::Closure<R>>::value
      ? '(' + ToString<R>::to_string() + ")*"
      : ToString<R>::to_string() + '*';
  }
};

template<>
struct ToString<impl::Omega> {
  static std::string to_string() {
    return "<o>";
  }
};

template<size_t I>
struct ToString<impl::Set<I>> {
  static std::string to_string() {
    return "<" + std::to_string(I) +">";
  }
};

template<typename Head, typename... Tails>
struct ToString<impl::Seq<Head, Tails...>> {
  static std::string to_string() {
    return ToString<Head>::to_string() + ToString<impl::Seq<Tails...>>::to_string();
  }
};

template<>
struct ToString<impl::Seq<>> {
  static std::string to_string() {
    return "";
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
struct TypeListPrinterImpl<impl::TypeList<impl::dfa::State<RE>, Remains...>, idx> {
  static void print() {
    printf("(%lu) %s %s\n", idx, (impl::dfa::State<RE>::accepting ? "(*)" : "   "), ToString<RE>::to_string().c_str());
    TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
  }
};

template<size_t idx, char C, typename... Remains>
struct TypeListPrinterImpl<impl::TypeList<impl::Char<C>, Remains...>, idx> {
  static void print() {
    printf("(%lu) %c\n", idx, C);
    TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
  }
};

template<size_t idx, size_t From, char C, size_t To, typename... Remains>
struct TypeListPrinterImpl<impl::TypeList<impl::dfa::Edge<From, C, To>, Remains...>, idx> {
  static void print() {
    printf("(%lu) %lu --%c-> %lu\n", idx, From, C, To);
    TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
  }
};

template<typename List> struct ActionTypeListToString;
template<typename Head, typename... Tails> struct ActionTypeListToString<impl::TypeList<Head, Tails...>> {
  static std::string to_string() {
    return ToString<Head>::to_string() + ", " + ActionTypeListToString<impl::TypeList<Tails...>>::to_string();
  }
};
template<> struct ActionTypeListToString<impl::TypeList<>> {
  static std::string to_string() { return ""; }
};

template<size_t idx, typename RE, typename... Remains>
struct TypeListPrinterImpl<impl::TypeList<impl::tnfa::State<RE>, Remains...>, idx> {
  static void print() {
    printf(
      "(%lu) %s %s : %s : { %s }\n",
      idx, (impl::tnfa::State<RE>::accepting ? "(*)" : "   "),
      ToString<typename impl::dfa::RemoveAllAction<RE>::type>::to_string().c_str(),
      ToString<RE>::to_string().c_str(),
      ActionTypeListToString<typename impl::tnfa::v<RE>::type>::to_string().c_str()
    );
    TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
  }
};

template<size_t idx, size_t From, char C, typename Action, size_t To, typename... Remains>
struct TypeListPrinterImpl<impl::TypeList<impl::tnfa::Edge<From, C, Action, To>, Remains...>, idx> {
  static void print() {
    printf("(%lu) %lu --%s, %c-> %lu\n", idx, From, ToString<Action>::to_string().c_str(), C, To);
    TypeListPrinterImpl<impl::TypeList<Remains...>, idx + 1>::print();
  }
};

template<size_t idx, typename Remain, typename Action, typename... Tails>
struct TypeListPrinterImpl<impl::TypeList<impl::tnfa::DerivedPair<Remain, Action>, Tails...>, idx> {
  static void print() {
    printf("(%lu) (%s, %s)\n", idx, ToString<Remain>::to_string().c_str(), ToString<Action>::to_string().c_str());
    TypeListPrinterImpl<impl::TypeList<Tails...>, idx + 1>::print();
  }
};

template <size_t idx>
struct TypeListPrinterImpl<impl::TypeList<>, idx> {
  static void print() {}
};
} /* namespace logger */
} /* namespace onre */

#endif /* !ONRE_LOGGER_HPP_ */
