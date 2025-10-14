#include "test_head.h"

void long_string_test() {
  std::cout << "\n=== Long String Tests (O(n) performance) ===\n";

  std::string long_a(N, 'a');
  test_match_and_log<"a*">(long_a, true);

  std::string long_ab(N, 'a');
  long_ab += 'b';
  test_match_and_log<"a*b">(long_ab, true);

  std::string long_alternating;
  for (int i = 0; i < N/2; i++) {
    long_alternating += "ab";
  }
  test_match_and_log<"(ab)*">(long_alternating, true);
  test_match_and_log<"(a?b)+">(long_alternating, true);

  std::string long_lower(N, 'm');
  test_match_and_log<"[a-z]+">(long_lower, true);

  std::string long_alnum;
  for (int i = 0; i < N; ++i) long_alnum += (i % 2 == 0) ? char('a' + (i % 26)) : char('0' + (i % 10));
  test_match_and_log<"[a-z0-9]+">(long_alnum, true);
}