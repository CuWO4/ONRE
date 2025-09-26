#include "test_head.h"

void non_matching_long_test() {
  std::cout << "\n=== Non-matching Long String Tests ===\n";
  std::string long_numbers(M, '1');
  TEST_AND_LOG("(a|b|c|d|e)*", long_numbers, false);
  TEST_AND_LOG("[a-e]*", long_numbers, false);
  TEST_AND_LOG(".*", long_numbers, true);

  std::string long_a_with_b(M, 'a');
  long_a_with_b[M/2] = 'b';
  TEST_AND_LOG("a*", long_a_with_b, false);
  TEST_AND_LOG("[a]*", long_a_with_b, false);
}