#include "test_head.h"

void notdotall_test() {
  std::cout << "\n=== NOT DOTALL Tests ===\n";
  test_match_and_log<".">("a", true);
  test_match_and_log<".">("\n", false);
  test_match_and_log<".">("\r", false);
  test_match_and_log<"a.b">("a\nb", false);
  test_match_and_log<"a.b">("a\rb", false);
}
