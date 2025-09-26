#include "test_head.h"

void character_class_test() {
  std::cout << "\n=== Character-class-heavy and Nested Tests ===\n";
  TEST_AND_LOG("([a-c][0-2])+", "a0b1", true);
  TEST_AND_LOG("([a-c][0-2])+", "a9", false);
  TEST_AND_LOG("([a-z]+[0-9]*)", "abc123", true);
  TEST_AND_LOG("([a-z]+[0-9]*)", "abc", true);
  TEST_AND_LOG("([a-z]+[0-9]*)", "123", false);
  TEST_AND_LOG("([ab]|[cd])*e", "ababcd", false);
  TEST_AND_LOG("([ab]|[cd])*e", "ababce", true);
}