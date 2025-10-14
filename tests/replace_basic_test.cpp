#include "test_head.h"

void replace_basic_test() {
  std::cout << "\n=== Replace Basic Tests ===\n";
  test_replace_and_log<"ab([ab]*)ab">("$0, $1, $$", "ababab", "ababab, ab, $");
  test_replace_and_log<"(ab)(cd)">("$2-$1", "abcd", "cd-ab");
  test_replace_and_log<"foo([0-9]+)">( "$0/$1", "foo123", "foo123/123");
  test_replace_and_log<"(hello)world">("$1", "helloworld", "hello");
  test_replace_and_log<"a(b)c">("$1$0", "abc", "babc");
  test_replace_and_log<"(.)">("$1", "x", "x");
  test_replace_and_log<"(ab|a)(c)">("$1|$2", "abc", "ab|c");
  test_replace_and_log<"(.)(.)">("$2$1", "xy", "yx");
  test_replace_and_log<"hello">("$0!", "hello", "hello!");
  test_replace_and_log<"(ab)(ab)(ab)">("$3|$2|$1", "ababab", "ab|ab|ab");
}