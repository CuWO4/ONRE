#include "test_head.h"

void replace_ambiguous_parsing_test() {
  std::cout << "\n=== Replace Ambiguous Parsing Tests ===\n";
  test_replace_and_log<"(a*)(a*)">("$1:$2", "aaaaa", "aaaaa:");
  test_replace_and_log<"(ab|a)(b)">( "$1+$2", "abb", "ab+b");
  test_replace_and_log<"(a|ab)(b)">( "$1+$2", "abb", "ab+b");
  test_replace_and_log<"(a?)(a?)">( "$1|$2", "a", "a|");
  test_replace_and_log<"(ab|a|aba)(b)">( "$1|$2", "abab", "aba|b");
  test_replace_and_log<"(a(b|ba))c">("$1-$2", "abac", "aba-ba");
  test_replace_and_log<"(a|ab)+b">("$1", "abab", "a");
}
