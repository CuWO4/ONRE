#include "test_head.h"

void character_class_test() {
  std::cout << "\n=== Character-class-heavy and Nested Tests ===\n";
  test_match_and_log<"([a-c][0-2])+">("a0b1", true);
  test_match_and_log<"([a-c][0-2])+">("a9", false);
  test_match_and_log<"([a-z]+[0-9]*)">("abc123", true);
  test_match_and_log<"([a-z]+[0-9]*)">("abc", true);
  test_match_and_log<"([a-z]+[0-9]*)">("123", false);
  test_match_and_log<"([ab]|[cd])*e">("ababcd", false);
  test_match_and_log<"([ab]|[cd])*e">("ababce", true);
  test_match_and_log<"([^ab])*e">("ababce", false);
  test_match_and_log<"([^ab])*a">("defga", true);
  test_match_and_log<"([^ab])*a">("defgaa", false);
}