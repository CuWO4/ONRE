#include "test_head.h"

void replace_deeply_nested_test() {
  std::cout << "\n=== Replace Deeply Nested Tests ===\n";
  test_replace_and_log<"(((a(b))c)d)">( "$0|$1|$2|$3|$4", "abcd", "abcd|abcd|abc|ab|b");
  test_replace_and_log<"((x(y(z))))">("$0:$1:$2:$3:$4", "xyz", "xyz:xyz:xyz:yz:z");
  test_replace_and_log<"(((ab)|a)b)c">("$0->$1", "abbc", "abbc->abb");
  test_replace_and_log<"((a|aa)|(aaa))(b)">("$1#$3@$4", "aaab", "aaa#aaa@b");
  test_replace_and_log<"(a(b(c(d(e)))))">("$5$4$3$2$1", "abcde", "edecdebcdeabcde");
  test_replace_and_log<"((p)(q)(r))(s)(t)">("$1|$4|$5|$6", "pqrst", "pqr|r|s|t");
  test_replace_and_log<"((1(2))3)4">("$0", "1234", "1234");
  test_replace_and_log<"((aa)(bb))(cc)">( "$2-$3-$4", "aabbcc", "aa-bb-cc");
  test_replace_and_log<"(((())))">("$0", "", "");
}
