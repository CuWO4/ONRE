#include "test_head.h"

void deeply_nested_test() {
  std::cout << "\n=== Deeply Nested and Chaotic Expressions ===\n";
  TEST_AND_LOG("((a|b)|(c|d))|(e|f)|(g|h)", "c", true);
  TEST_AND_LOG("((a|b)|(c|d))|(e|f)|(g|h)", "h", true);
  TEST_AND_LOG("((a|b)|(c|d))|(e|f)|(g|h)", "x", false);
  TEST_AND_LOG("(((a)|(b))*)", "abab", true);
  TEST_AND_LOG("(((a)|(b))*)", "aaabbb", true);

  TEST_AND_LOG("((a|[bcd])|(c|d))|(e|f)", "d", true);
  TEST_AND_LOG("(a([bcd])*)*", "abcbcd", true);

  TEST_AND_LOG("(a(b)*)*", "abab", true);
  TEST_AND_LOG("(a(b)*)*", "a", true);
  TEST_AND_LOG("(a(b)*)*", "abb", true);
  TEST_AND_LOG("(a(b)*)*", "b", false);

  TEST_AND_LOG("a*b+", "b", true);
  TEST_AND_LOG("a*b+", "ab", true);
  TEST_AND_LOG("a*b+", "aaabbb", true);
  TEST_AND_LOG("a*b+", "a", false);
}