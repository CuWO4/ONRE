#include "test_head.h"

void replace_non_capturing_group_test() {
  std::cout << "\n=== Replace Non-Capturing Group Tests ===\n";
  test_replace_and_log<"a(?:b|c)d">("$0", "abd", "abd");
  test_replace_and_log<"(?:ab)(cd)">("$1", "abcd", "cd");
  test_replace_and_log<"(a(?:b|c)d)e">("$1", "abde", "abd");
  test_replace_and_log<"x(?:y(z))w">("$1", "xyzw", "z");
  test_replace_and_log<"((?:ab)+)(c)">("$1-$2", "abababc", "ababab-c");
}
