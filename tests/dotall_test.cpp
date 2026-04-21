#define ONRE_DOTALL
#include "test_head.h"

void dotall_test() {
  std::cout << "\n=== DOTALL Tests ===\n";
  test_match_and_log<".">("a", true);
  test_match_and_log<".">("\n", true);
  test_match_and_log<".">("\r", true);
  test_match_and_log<"a.b">("a\nb", true);
  test_match_and_log<"a.b">("a\rb", true);
  test_replace_and_log<"a.b">("$0", "a\nb", "a\nb");
}
