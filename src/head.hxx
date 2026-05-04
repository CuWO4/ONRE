#ifndef HEAD_HXX_xC8ZmzgF_
#define HEAD_HXX_xC8ZmzgF_

// === snippet begin ===
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

[[maybe_unused]] static inline void dummy_check_global_xC8ZmzgF_() {
  constexpr auto count_double_colon = [](const char* str) {
    unsigned cnt = 0;
    for (char const* p = str; *p && *(p+1); p++)
      cnt += *p == ':' && *(p+1) == ':';
    return cnt;
  };

  static_assert(
    #if defined(__clang__) || defined(__GNUC__)
      count_double_colon(__PRETTY_FUNCTION__) == 0,
    #elif defined(_MSC_VER)
      count_double_colon(__FUNCSIG__) == 0,
    #else
      true,
    #endif
    "header is not placed in global namespace"
  );
}

// === snippet end ===

#endif
