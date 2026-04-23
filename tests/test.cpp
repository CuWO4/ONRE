#include "test_head.h"

int main(int argc, char** argv) {
  bool verbose = false;
  for (int i = 1; i < argc; i++) {
    if (std::string_view(argv[i]) == "-v") verbose = true;
  }

  reset_test_counters();
  set_test_verbose(verbose);

  std::streambuf* saved_cout = nullptr;
  if (!verbose) saved_cout = std::cout.rdbuf(g_test_null_stream.rdbuf());

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
  quantifier_test();
  divisible_test();
  notdotall_test();
  dotall_test();

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
  replace_quantifier_test();
  replace_longest_match_test();
  replace_non_capturing_group_test();
  utf8_test();

  if (!verbose) std::cout.rdbuf(saved_cout);

  print_test_summary();

  return g_test_passed == g_test_total ? 0 : 1;
}
