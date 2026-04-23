#include "test_head.h"

void comment_test() {
  std::cout << "\n=== Comment Tests ===\n";

  test_match_and_log<"a(?#ignore)b">("ab", true);
  test_match_and_log<"(?#leading)ab">("ab", true);
  test_match_and_log<"a(?#middle)(b|c)">("ab", true);
  test_match_and_log<"a(?#middle)(b|c)">("ac", true);
  test_match_and_log<"(a(?#inner)b)c">("abc", true);
  test_match_and_log<"(a|b)(?#alt)c">("ac", true);
  test_match_and_log<"(a|b)(?#alt)c">("bc", true);
  test_match_and_log<"a(?#tail)b*">("abbb", true);
  test_match_and_log<"é(?#注释)😀">((char const*)u8"é😀", true);
  test_match_and_log<"a(?#注释)b">("ac", false);
}