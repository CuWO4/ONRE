#ifndef FIXEDSTRING_HXX__
#define FIXEDSTRING_HXX__

#include "head.hxx"

// === snippet begin ===
namespace onre {
namespace impl {

/* === fixed string, a string container enabling compile-time visiting === */
template<size_t N>
struct FixedString {
  uint8_t data[N]; /* include '\0' */

  constexpr FixedString(const char (&str)[N]) {
    for (size_t i = 0; i < N; i++)
      data[i] = str[i];
  }
  constexpr FixedString(const char8_t (&str)[N]) {
    for (size_t i = 0; i < N; i++)
      data[i] = str[i];
  }

  constexpr FixedString(const FixedString&) noexcept = default;
  constexpr FixedString(FixedString&&) noexcept = default;
  constexpr FixedString& operator=(const FixedString&) noexcept = default;
  constexpr FixedString& operator=(FixedString&&) noexcept = default;

  static constexpr size_t length = N - 1;
  constexpr const char* c_str() const {
    return (char const*) data; // implementation defined behavior, but on almost all modern system, it works.
  }
  constexpr uint8_t operator[](size_t i) const {
    return data[i];
  }
};
template<size_t N>
FixedString(const char (&str)[N]) -> FixedString<N>;
template<size_t N>
FixedString(const char8_t (&str)[N]) -> FixedString<N>;

} /* namespace impl */
} /* namespace onre */

// === snippet end ===

#endif