#include "test_head.h"

void replace_boundary_test() {
  std::cout << "\n=== Replace Boundary Tests ===\n";
  test_replace_and_log<"">("", "", "");
  test_replace_and_log<"">("$0", "", "");
  test_replace_and_log<"">("123", "", "123");
  test_replace_and_log<"a|">("$0", "", "");
  test_replace_and_log<"a|">("$0", "a", "a");
  test_replace_and_log<"a*">("", "aaaa", "");
}