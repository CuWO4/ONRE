#include "test_head.h"

void extended_alphabet_and_escape_test() {
  std::cout << "\n=== Extended Alphabet & Escape Tests ===\n";
  TEST_AND_LOG("([a-e]+)@([a-e]+)[.]com", "abcc@de.com", true);
  TEST_AND_LOG("([a-e]+)@([a-e]+)[.]com", "abcc@de", false);
  TEST_AND_LOG("([a-e]+)@([a-e]+)\\.com", "abcc@de.com", true);
  TEST_AND_LOG("([a-e]+)@([a-e]+)\\.com", "abcc@de", false);
  TEST_AND_LOG("((ab)*|c*|b)(@\\.)?", "abab", true);
  TEST_AND_LOG("((ab)*|c*|b)(@\\.)?", "abab@.", true);
  TEST_AND_LOG("((ab)*|c*|b)(@\\.)?", "ababc", false);
}