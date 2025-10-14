#include "test_head.h"

void replace_longest_match_test() {
  std::cout << "\n=== Replace Longest Match Tests ===\n";
  test_replace_and_log<"(a*)b">("$1", "aaaaab", "aaaaa");
  test_replace_and_log<"(.*)end">("$1", "startmiddleend", "startmiddle");
  test_replace_and_log<"(ab|a)b">("$1", "abb", "ab");
  test_replace_and_log<"(a+)(a*)">("$1|$2", "aaaa", "aaaa|");
  test_replace_and_log<"(a*)(a*)">("$1|$2", "aaaa", "aaaa|");
  test_replace_and_log<"(a+)(a+)">("$1|$2", "aaaa", "aaa|a");
  test_replace_and_log<"(a*)(a+)">("$1|$2", "aaaa", "aaa|a");
  test_replace_and_log<"(a|aa|aaa)a">("$0:$1", "aaaa", "aaaa:aaa");
  test_replace_and_log<"(ab|aba)(b)">("$1-$2", "abab", "aba-b");
  test_replace_and_log<"((a*)(a*)b)*">("$1-$2-$3", "aaaab", "aaaab-aaaa-");
  test_replace_and_log<"((a*)(a*)b)*">("$1-$2-$3", "aaaabaab", "aaaab-aaaa-");
  test_replace_and_log<"((a*)(a*)b)*">("$1-$2-$3", "aabaaaab", "aaaab-aaaa-");
  test_replace_and_log<"((a*)(a*)b)*">("$1-$2-$3", "aabaaaabaabb", "aaaab-aaaa-");
}
