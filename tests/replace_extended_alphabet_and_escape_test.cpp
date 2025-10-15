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
  test_replace_and_log<"\\d*">("$0", "123", "123");
  test_replace_and_log<"\\D+">("$0", "aaas", "aaas");
  test_replace_and_log<"\\s+">("$0", " \n\r\f\t", " \n\r\f\t");
  test_replace_and_log<"\\S+">("$0", "aalfqwe13__#@'", "aalfqwe13__#@'");
  test_replace_and_log<"(\\w+)\\s+(\\w+)">("$1:$2", "hello10 _world", "hello10:_world");
  test_replace_and_log<"\\n">("$0", "\n", "\n");
  test_replace_and_log<"\\t">("$0", "\t", "\t");
  test_replace_and_log<"\\f">("$0", "\f", "\f");
  test_replace_and_log<"\\r">("$0", "\r", "\r");
  test_replace_and_log<"\\x4f">("$0", "O", "O");
  test_replace_and_log<"[\\s\\x4f]+">("$0", "O  O \n", "O  O \n");
}
