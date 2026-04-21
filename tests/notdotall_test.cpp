#include "test_head.h"

void notdotall_test() {
  std::cout << "\n=== NOT DOTALL Tests ===\n";
  test_match_and_log<"2.">("2a", true);
  test_match_and_log<"2.">("2\n", false);
  test_match_and_log<"2.">("2\r", false);
  test_match_and_log<"2a.b">("2a\nb", false);
  test_match_and_log<"2a.b">("2a\rb", false);
}
