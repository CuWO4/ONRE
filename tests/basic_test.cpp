#include "test_head.h"

void basic_test() {
  std::cout << "\n=== Basic Tests ===\n";
  TEST_AND_LOG("a", "a", true);
  TEST_AND_LOG("a", "b", false);
  TEST_AND_LOG("a|b", "a", true);
  TEST_AND_LOG("a|b", "b", true);
  TEST_AND_LOG("ab", "ab", true);
  TEST_AND_LOG("ab", "a", false);

  TEST_AND_LOG("a*", "", true);
  TEST_AND_LOG("a*", "a", true);
  TEST_AND_LOG("a*", "aa", true);
  TEST_AND_LOG("a+", "a", true);
  TEST_AND_LOG("a+", "", false);
  TEST_AND_LOG("a?", "", true);
  TEST_AND_LOG("a?", "a", true);

  TEST_AND_LOG("(ab)*", "", true);
  TEST_AND_LOG("(ab)*", "abab", true);
  TEST_AND_LOG("(ab)+", "ab", true);
  TEST_AND_LOG("(ab)+", "a", false);
  TEST_AND_LOG("a*b", "aaab", true);
  TEST_AND_LOG("a*b", "b", true);
  TEST_AND_LOG("a+b", "aaab", true);

  TEST_AND_LOG("(a|b)*|c", "c", true);
  TEST_AND_LOG("(a|b)*|c", "abb", true);
  TEST_AND_LOG("(a|b)*|c", "d", false);

  TEST_AND_LOG("[abc]", "b", true);
  TEST_AND_LOG("[abc]", "d", false);
  TEST_AND_LOG("[a-z]", "m", true);
  TEST_AND_LOG("[a-z]", "A", false);
  TEST_AND_LOG("[0-9]", "5", true);
  TEST_AND_LOG("[0-9]", "a", false);
  TEST_AND_LOG(".", "5", true);
  TEST_AND_LOG(".", "a", true);
  TEST_AND_LOG("..", "ab", true);
  TEST_AND_LOG("..", "a", false);
}