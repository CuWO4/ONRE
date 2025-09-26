#include "test_head.h"

void ambiguous_parsing_test() {
  std::cout << "\n=== Ambiguous and Tricky Parsing Tests ===\n";
  TEST_AND_LOG("a|b|c", "b", true);
  TEST_AND_LOG("a|b|c", "d", false);
  TEST_AND_LOG("(a|b)|c", "c", true);
  TEST_AND_LOG("a|(b|c)", "c", true);
  TEST_AND_LOG("a||b", "a", true);
  TEST_AND_LOG("a||b", "b", true);
  TEST_AND_LOG("a||b", "", true);
  TEST_AND_LOG("a||b", "c", false);

  TEST_AND_LOG("(a*|b*)*", "", true);
  TEST_AND_LOG("(a*|b*)*", "a", true);
  TEST_AND_LOG("(a*|b*)*", "b", true);
  TEST_AND_LOG("(a*|b*)*", "abba", true);
  TEST_AND_LOG("(a*|b*)*", "c", false);
}