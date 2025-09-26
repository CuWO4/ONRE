#include "test_head.h"

void long_string_test() {
  std::cout << "\n=== Long String Tests (O(n) performance) ===\n";

  std::string long_a(N, 'a');
  TEST_AND_LOG("a*", long_a, true);

  std::string long_ab(N, 'a');
  long_ab += 'b';
  TEST_AND_LOG("a*b", long_ab, true);

  std::string long_alternating;
  for (int i = 0; i < N/2; i++) {
    long_alternating += "ab";
  }
  TEST_AND_LOG("(ab)*", long_alternating, true);
  TEST_AND_LOG("(a?b)+", long_alternating, true);

  std::string long_lower(N, 'm');
  TEST_AND_LOG("[a-z]+", long_lower, true);

  std::string long_alnum;
  for (int i = 0; i < N; ++i) long_alnum += (i % 2 == 0) ? char('a' + (i % 26)) : char('0' + (i % 10));
  TEST_AND_LOG("[a-z0-9]+", long_alnum, true);
}