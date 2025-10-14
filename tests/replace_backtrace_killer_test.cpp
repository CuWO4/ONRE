#include "test_head.h"

void replace_backtrace_killer_test() {
  std::cout << "\n=== Replace Backtrace Killer Tests ===\n";
  test_replace_and_log<"(a+)+b">("$0", std::string(M, 'a') + "b", std::string(M, 'a') + "b");
  test_replace_and_log<"(a?)+b">("$0", std::string(M, 'a') + "b", std::string(M, 'a') + "b");
  test_replace_and_log<"(a|b)+c">("$0", std::string(M, 'a') + "c", std::string(M, 'a') + "c");
  test_replace_and_log<"(ab|a)*c">("$0", std::string(M, 'a') + "c", std::string(M, 'a') + "c");
  test_replace_and_log<"(a+)(a+)b">("$1|$2", std::string(M, 'a') + "b", std::string(M - 1, 'a') + "|a");
  test_replace_and_log<"(a|ab)+b">("OK", std::string(M, 'a') + "b", "OK");
  test_replace_and_log<"(a*)b">("$1B", std::string(M, 'a') + "b", std::string(M, 'a') + "B");
}
