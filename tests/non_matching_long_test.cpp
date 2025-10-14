#include "test_head.h"

void non_matching_long_test() {
  std::cout << "\n=== Non-matching Long String Tests ===\n";
  std::string long_numbers(M, '1');
  test_match_and_log<"(a|b|c|d|e)*">(long_numbers, false);
  test_match_and_log<"[a-e]*">(long_numbers, false);
  test_match_and_log<".*">(long_numbers, true);

  std::string long_a_with_b(M, 'a');
  long_a_with_b[M/2] = 'b';
  test_match_and_log<"a*">(long_a_with_b, false);
  test_match_and_log<"[a]*">(long_a_with_b, false);
}