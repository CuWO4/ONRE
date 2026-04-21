#define ONRE_DOTALL
#include "test_head.h"

void dotall_test() {
  std::cout << "\n=== DOTALL Tests ===\n";
  test_match_and_log<"1.">("1a", true);
  test_match_and_log<"1.">("1\n", true);
  test_match_and_log<"1.">("1\r", true);
  test_match_and_log<"1a.b">("1a\nb", true);
  test_match_and_log<"1a.b">("1a\rb", true);
  test_replace_and_log<"1a.b">("$0", "1a\nb", "1a\nb");
}
