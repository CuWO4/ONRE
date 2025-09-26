#include "test_head.h"

void backtrace_killer_test() {
  std::cout << "\n=== Backtracking Killer Tests ===\n";
  std::string no_c(M, 'a');
  TEST_AND_LOG("(a|b)*c", no_c, false);

  std::string almost_two_a(M, 'b');
  almost_two_a += 'a';
  TEST_AND_LOG("(a|b)*a(a|b)*a", almost_two_a, false);

  std::string long_a_simple(M, 'a');
  TEST_AND_LOG("(a*)*", long_a_simple, true);

  std::string long_b(M, 'b');
  TEST_AND_LOG("(a*(b*)*)*", long_b, true);

  std::string prefix(M, 'a');
  std::string suffix(M, 'b');
  TEST_AND_LOG("(a|b)*a(a|b)*b", prefix + "c" + suffix, false);

  std::string long_letters(M, 'x');
  TEST_AND_LOG("([a-z])*z", long_letters, false);
}