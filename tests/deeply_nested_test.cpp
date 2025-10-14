#include "test_head.h"

void deeply_nested_test() {
  std::cout << "\n=== Deeply Nested and Chaotic Expressions ===\n";
  test_match_and_log<"((a|b)|(c|d))|(e|f)|(g|h)">("c", true);
  test_match_and_log<"((a|b)|(c|d))|(e|f)|(g|h)">("h", true);
  test_match_and_log<"((a|b)|(c|d))|(e|f)|(g|h)">("x", false);
  test_match_and_log<"(((a)|(b))*)">("abab", true);
  test_match_and_log<"(((a)|(b))*)">("aaabbb", true);

  test_match_and_log<"((a|[bcd])|(c|d))|(e|f)">("d", true);
  test_match_and_log<"(a([bcd])*)*">("abcbcd", true);

  test_match_and_log<"(a(b)*)*">("abab", true);
  test_match_and_log<"(a(b)*)*">("a", true);
  test_match_and_log<"(a(b)*)*">("abb", true);
  test_match_and_log<"(a(b)*)*">("b", false);

  test_match_and_log<"a*b+">("b", true);
  test_match_and_log<"a*b+">("ab", true);
  test_match_and_log<"a*b+">("aaabbb", true);
  test_match_and_log<"a*b+">("a", false);
}