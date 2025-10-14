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
  divisible_test();

  replace_basic_test();
  replace_boundary_test();
  replace_long_string_test();
  replace_backtrace_killer_test();
  replace_deeply_nested_test();
  replace_ambiguous_parsing_test();
  replace_chaotic_mixed_test();
  replace_long_string_complex_test();
  replace_character_class_test();
  replace_extended_alphabet_and_escape_test();
  replace_longest_match_test();

  return 0;
}
