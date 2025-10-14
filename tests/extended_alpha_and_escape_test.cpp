#include "test_head.h"

void extended_alphabet_and_escape_test() {
  std::cout << "\n=== Extended Alphabet & Escape Tests ===\n";
  test_match_and_log<"([a-e]+)@([a-e]+)[.]com">("abcc@de.com", true);
  test_match_and_log<"([a-e]+)@([a-e]+)[.]com">("abcc@de", false);
  test_match_and_log<"([a-e]+)@([a-e]+)\\.com">("abcc@de.com", true);
  test_match_and_log<"([a-e]+)@([a-e]+)\\.com">("abcc@de", false);
  test_match_and_log<"((ab)*|c*|b)(@\\.)?">("abab", true);
  test_match_and_log<"((ab)*|c*|b)(@\\.)?">("abab@.", true);
  test_match_and_log<"((ab)*|c*|b)(@\\.)?">("ababc", false);
}