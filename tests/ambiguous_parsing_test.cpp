#include "test_head.h"

void ambiguous_parsing_test() {
  std::cout << "\n=== Ambiguous and Tricky Parsing Tests ===\n";
  test_match_and_log<"a|b|c">("b", true);
  test_match_and_log<"a|b|c">("d", false);
  test_match_and_log<"(a|b)|c">("c", true);
  test_match_and_log<"a|(b|c)">("c", true);
  test_match_and_log<"a||b">("a", true);
  test_match_and_log<"a||b">("b", true);
  test_match_and_log<"a||b">("", true);
  test_match_and_log<"a||b">("c", false);

  test_match_and_log<"(a*|b*)*">("", true);
  test_match_and_log<"(a*|b*)*">("a", true);
  test_match_and_log<"(a*|b*)*">("b", true);
  test_match_and_log<"(a*|b*)*">("abba", true);
  test_match_and_log<"(a*|b*)*">("c", false);
}