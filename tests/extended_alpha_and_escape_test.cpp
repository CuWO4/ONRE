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
  test_match_and_log<"\\d">("1", true);
  test_match_and_log<"\\d">("af", false);
  test_match_and_log<"\\d">("123", false);
  test_match_and_log<"\\d*">("123", true);
  test_match_and_log<"\\d*">("12a3", false);
  test_match_and_log<"\\D">("a", true);
  test_match_and_log<"\\D">("1", false);
  test_match_and_log<"\\D">("agf", false);
  test_match_and_log<"\\D+">("aaas", true);
  test_match_and_log<"\\D*">("af13h", false);
  test_match_and_log<"\\s">(" ", true);
  test_match_and_log<"\\s+">(" \n\r\f\t", true);
  test_match_and_log<"\\s+">(" \n\r\fa\t", false);
  test_match_and_log<"\\S">("a", true);
  test_match_and_log<"\\S+">("aalfqwe13__#@'", true);
  test_match_and_log<"\\w">("a", true);
  test_match_and_log<"\\w+">("hello10_world", true);
  test_match_and_log<"\\w+\\s+\\w+">("hello10 _world", true);
  test_match_and_log<"\\w+\\s+\\w+">("hello10_world", false);
  test_match_and_log<"\\n">("\n", true);
  test_match_and_log<"\\t">("\t", true);
  test_match_and_log<"\\f">("\f", true);
  test_match_and_log<"\\r">("\r", true);
  test_match_and_log<"\\x4F">("OOO", false);
  test_match_and_log<"\\x4f">("O", true);
  test_match_and_log<"[\\s\\x4f]+">("O  O \n", true);
  test_match_and_log<"[\\s\\x4f]+">("O  Or \n", false);
}