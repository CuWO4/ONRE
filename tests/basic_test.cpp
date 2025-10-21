#include "test_head.h"

void basic_test() {
  std::cout << "\n=== Basic Tests ===\n";
  test_match_and_log<"a">("a", true);
  test_match_and_log<"a">("b", false);
  test_match_and_log<"a|b">("a", true);
  test_match_and_log<"a|b">("b", true);
  test_match_and_log<"ab">("ab", true);
  test_match_and_log<"ab">("a", false);

  test_match_and_log<"a*">("", true);
  test_match_and_log<"a*">("a", true);
  test_match_and_log<"a*">("aa", true);
  test_match_and_log<"a+">("a", true);
  test_match_and_log<"a+">("", false);
  test_match_and_log<"a?">("", true);
  test_match_and_log<"a?">("a", true);

  test_match_and_log<"(ab)*">("", true);
  test_match_and_log<"(ab)*">("abab", true);
  test_match_and_log<"(ab)+">("ab", true);
  test_match_and_log<"(ab)+">("a", false);
  test_match_and_log<"a*b">("aaab", true);
  test_match_and_log<"a*b">("b", true);
  test_match_and_log<"a+b">("aaab", true);

  test_match_and_log<"(a|b)*|c">("c", true);
  test_match_and_log<"(a|b)*|c">("abb", true);
  test_match_and_log<"(a|b)*|c">("d", false);

  test_match_and_log<"[abc]">("b", true);
  test_match_and_log<"[abc]">("d", false);
  test_match_and_log<"[a-z]">("m", true);
  test_match_and_log<"[a-z]">("A", false);
  test_match_and_log<"[0-9]">("5", true);
  test_match_and_log<"[0-9]">("a", false);
  test_match_and_log<"[^0-9]">("a", true);
  test_match_and_log<"[^0-9]">("4", false);
  test_match_and_log<"[^abc]">("a", false);
  test_match_and_log<"[^abc]">("d", true);
  test_match_and_log<".">("5", true);
  test_match_and_log<".">("a", true);
  test_match_and_log<"..">("ab", true);
  test_match_and_log<"..">("a", false);
  test_match_and_log<".*">("aad12*(1)", true);
  test_match_and_log<".*">("aad1\n2*(1)", false);
}