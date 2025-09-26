#include "test_head.h"

int main() {
  basic_test();
  boundary_test();
  long_string_test();
  backtrace_killer_test();
  mixed_character_test();
  non_matching_long_test();
  deeply_nested_test();
  ambiguous_parsing_test();
  chaotic_mixed_test();
  long_string_complex_test();
  character_class_test();
  extended_alphabet_and_escape_test();

  return 0;
}
