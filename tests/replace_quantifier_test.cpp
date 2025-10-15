#include "test_head.h"

void replace_quantifier_test() {
  std::cout << "\n=== Replace Quantifier Test ===\n";
  test_replace_and_log<"(ab){3}">("$0", "ababab", "ababab");
  test_replace_and_log<"(ab){3,}">("$0", "ababab", "ababab");
  test_replace_and_log<"(ab){3,}">("$0", "abababab", "abababab");
  test_replace_and_log<"(ab){3,}">("$0", "ababababab", "ababababab");
  test_replace_and_log<"(ab){,3}">("$0", "", "");
  test_replace_and_log<"(ab){,3}">("$0", "ab", "ab");
  test_replace_and_log<"(ab){,3}">("$0", "abab", "abab");
  test_replace_and_log<"(ab){,3}">("$0", "ababab", "ababab");
  test_replace_and_log<"(ab){1,3}">("$0", "ab", "ab");
  test_replace_and_log<"(ab){1,3}">("$0", "abab", "abab");
  test_replace_and_log<"(ab){1,3}">("$0", "ababab", "ababab");
}