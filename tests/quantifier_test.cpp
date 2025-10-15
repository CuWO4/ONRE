#include "test_head.h"

void quantifier_test() {
  std::cout << "\n=== Quantifier Test ===\n";
  test_match_and_log<"(ab){3}">("ababab", true);
  test_match_and_log<"(ab){3}">("", false);
  test_match_and_log<"(ab){3}">("ab", false);
  test_match_and_log<"(ab){3}">("abababa", false);
  test_match_and_log<"(ab){3}">("abababab", false);
  test_match_and_log<"(ab){3,}">("", false);
  test_match_and_log<"(ab){3,}">("abab", false);
  test_match_and_log<"(ab){3,}">("ababab", true);
  test_match_and_log<"(ab){3,}">("abababab", true);
  test_match_and_log<"(ab){3,}">("ababababab", true);
  test_match_and_log<"(ab){,3}">("", true);
  test_match_and_log<"(ab){,3}">("ab", true);
  test_match_and_log<"(ab){,3}">("abab", true);
  test_match_and_log<"(ab){,3}">("ababab", true);
  test_match_and_log<"(ab){,3}">("abababab", false);
  test_match_and_log<"(ab){,3}">("ababababab", false);
  test_match_and_log<"(ab){1,3}">("", false);
  test_match_and_log<"(ab){1,3}">("ab", true);
  test_match_and_log<"(ab){1,3}">("abab", true);
  test_match_and_log<"(ab){1,3}">("ababab", true);
  test_match_and_log<"(ab){1,3}">("abababab", false);
  test_match_and_log<"(ab){1,3}">("ababababab", false);
}