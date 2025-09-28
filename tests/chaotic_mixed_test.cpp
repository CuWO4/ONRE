#include "test_head.h"

void chaotic_mixed_test() {
  std::cout << "\n=== Chaotic Mixed Expressions ===\n";
  TEST_AND_LOG("((a|b)(c|d))*|(e|f)(g|h)*", "ac", true);
  TEST_AND_LOG("((a|b)(c|d))*|(e|f)(g|h)*", "eggg", true);
  TEST_AND_LOG("((a|b)(c|d))*|(e|f)(g|h)*", "bd", true);
  TEST_AND_LOG("((a|b)(c|d))*|(e|f)(g|h)*", "acbd", true);
  TEST_AND_LOG("((a|b)(c|d))*|(e|f)(g|h)*", "x", false);
  TEST_AND_LOG("(a|(b|c))*d", "aaad", true);
  TEST_AND_LOG("(a|(b|c))*d", "abcd", true);
  TEST_AND_LOG("(a|(b|c))*d", "abca", false);
  TEST_AND_LOG("(a|[bcd]|(e|f))", "e", true);
}