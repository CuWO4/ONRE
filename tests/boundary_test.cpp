#include "test_head.h"

void boundary_test() {
  std::cout << "\n=== Boundary Tests ===\n";
  test_match_and_log<"">("", true);
  test_match_and_log<"">("a", false);
  test_match_and_log<"a">("", false);
  test_match_and_log<"(a|)">("", true);
  test_match_and_log<"(a|)">("a", true);
  test_match_and_log<"[a]*">("", true);
  test_match_and_log<"[a]">("", false);
}