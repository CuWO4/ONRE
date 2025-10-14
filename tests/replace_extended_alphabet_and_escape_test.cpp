#include "test_head.h"

void replace_extended_alphabet_and_escape_test() {
  std::cout << "\n=== Replace Extended Alphabet and Escape Tests ===\n";
  test_replace_and_log<"a\\.b">("DOT", "a.b", "DOT");
  test_replace_and_log<"\\\\">("BS", "\\", "BS");
  test_replace_and_log<"\\*star\\*">("S", "*star*", "S");
  test_replace_and_log<"\\+plus\\+">("P", "+plus+", "P");
  test_replace_and_log<"\\(paren\\)">( "P", "(paren)", "P");
  test_replace_and_log<"slash\\/" >("S", "slash/", "S");
  test_replace_and_log<"col:on">("C", "col:on", "C");
  test_replace_and_log<"question\\?">("Q", "question?", "Q");
}
