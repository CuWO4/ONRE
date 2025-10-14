#include "test_head.h"

void replace_character_class_test() {
  std::cout << "\n=== Replace Character Class Tests ===\n";
  test_replace_and_log<"[0-9]+">("NUM", "12345", "NUM");
  test_replace_and_log<"[a-zA-Z]+">("WORD", "Hello", "WORD");
  test_replace_and_log<"[^0-9]+">("NOTNUM", "abcXYZ", "NOTNUM");
  test_replace_and_log<"[a-c]+">("$0!", "abccba", "abccba!");
  test_replace_and_log<"[\\[\\]]+">("BRACK", "[]", "BRACK");
  test_replace_and_log<"[A-Z][a-z]+">("$0", "Camel", "Camel");
}
