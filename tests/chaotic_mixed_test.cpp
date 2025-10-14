#include "test_head.h"

void chaotic_mixed_test() {
  std::cout << "\n=== Chaotic Mixed Expressions ===\n";
  test_match_and_log<"((a|b)(c|d))*|(e|f)(g|h)*">("ac", true);
  test_match_and_log<"((a|b)(c|d))*|(e|f)(g|h)*">("eggg", true);
  test_match_and_log<"((a|b)(c|d))*|(e|f)(g|h)*">("bd", true);
  test_match_and_log<"((a|b)(c|d))*|(e|f)(g|h)*">("acbd", true);
  test_match_and_log<"((a|b)(c|d))*|(e|f)(g|h)*">("x", false);
  test_match_and_log<"(a|(b|c))*d">("aaad", true);
  test_match_and_log<"(a|(b|c))*d">("abcd", true);
  test_match_and_log<"(a|(b|c))*d">("abca", false);
  test_match_and_log<"(a|[bcd]|(e|f))">("e", true);
}