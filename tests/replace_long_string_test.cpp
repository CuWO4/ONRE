#include "test_head.h"

void replace_long_string_test() {
  std::cout << "\n=== Replace Long String Tests ===\n";
  std::string long_a(N, 'a');
  std::string long_b(N, 'b');
  test_replace_and_log<"a+Xb+">("$0mid$0", long_a + "X" + long_b, long_a + "X" + long_b + "mid" + long_a + "X" + long_b);
  test_replace_and_log<"(x*)end">("$1-END", std::string(M, 'x') + "end", std::string(M, 'x') + "-END");
  test_replace_and_log<"start(x*)end">("$1", "start" + std::string(M, 'x') + "end", std::string(M, 'x'));
  test_replace_and_log<"(a|b)+">("$0", long_a + long_b + long_a, long_a + long_b + long_a);
}
