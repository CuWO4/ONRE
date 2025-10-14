#include "test_head.h"

void replace_long_string_complex_test() {
  std::cout << "\n=== Replace Long String Complex Pattern Tests ===\n";
  std::string long_mixed_ab;
  for (int i = 0; i < N; i++) {
      long_mixed_ab += (i % 2 == 0) ? 'a' : 'b';
  }
  test_replace_and_log<"(a|b)*">("$0:$1", long_mixed_ab, long_mixed_ab + ":" + *long_mixed_ab.rbegin());
  test_replace_and_log<"(a*b*)*">("$0", long_mixed_ab, long_mixed_ab);
  std::string long_as = std::string(N / 2, 'a');
  std::string long_as_with_b = long_as + "b" + long_as;
  test_replace_and_log<"(a*)b(a*)">("$0:$1:$2", long_as_with_b, long_as_with_b + ":" + long_as + ":" + long_as);
  test_replace_and_log<"prefix((ab?)+)suffix">("$1", "prefix" + long_mixed_ab + "suffix", long_mixed_ab);
  test_replace_and_log<"(a+b?)*">("$0", long_mixed_ab, long_mixed_ab);
  test_replace_and_log<"(a?b?)*">("$0", long_mixed_ab, long_mixed_ab);
  test_replace_and_log<"(.*)">("$0:$1", long_mixed_ab, long_mixed_ab + ":" + long_mixed_ab);
}
