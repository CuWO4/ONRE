#include "test_head.h"

void backtrace_killer_test() {
  std::cout << "\n=== Backtracking Killer Tests ===\n";
  std::string no_c(M, 'a');
  test_match_and_log<"(a|b)*c">(no_c, false);

  std::string almost_two_a(M, 'b');
  almost_two_a += 'a';
  test_match_and_log<"(a|b)*a(a|b)*a">(almost_two_a, false);

  std::string long_a_simple(M, 'a');
  test_match_and_log<"(a*)*">(long_a_simple, true);

  std::string long_b(M, 'b');
  test_match_and_log<"(a*(b*)*)*">(long_b, true);

  std::string prefix(M, 'a');
  std::string suffix(M, 'b');
  test_match_and_log<"(a|b)*a(a|b)*b">(prefix + "c" + suffix, false);

  std::string long_letters(M, 'x');
  test_match_and_log<"([a-z])*z">(long_letters, false);
}