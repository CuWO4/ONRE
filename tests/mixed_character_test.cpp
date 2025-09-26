#include "test_head.h"

void mixed_character_test() {
  std::cout << "\n=== Mixed Character Tests ===\n";
  std::string mixed;
  for (int i = 0; i < M; i++) {
    mixed += ('a' + (i % 26));
  }
  TEST_AND_LOG("(a|b|c|d|e)*", mixed, false);
  TEST_AND_LOG("[abcde]*", mixed, false);
}