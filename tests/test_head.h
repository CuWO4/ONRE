#ifndef TEST_HEAD_H_
#define TEST_HEAD_H_

#include "../regex.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

#define TEST_AND_LOG(pattern, str, expected) \
do { \
  auto start = std::chrono::high_resolution_clock::now(); \
  bool result = onre::Regex<pattern>::is_match(str); \
  auto end = std::chrono::high_resolution_clock::now(); \
  std::string s(str); \
  const char* color = (result == (expected)) ? "\033[1;32m" : "\033[1;31m"; \
  const char* reset = "\033[0m"; \
  std::string pattern_s(pattern); \
  std::cout << "pattern: " << std::left << std::setw(40) \
            << (pattern_s.length() > 35 ? pattern_s.substr(0, 15) + "...." + pattern_s.substr(pattern_s.length() - 15) : pattern_s) \
            << " pattern_len: " << std::setw(5) << pattern_s.length() \
            << " str: " << std::setw(25) \
            << (s.length() > 20 ? s.substr(0, 8) + "...." + s.substr(s.length() - 8) : s) \
            << " str_len: " << std::setw(8) << std::to_string(s.length()) \
            << " result: " << color << std::setw(6) << (result ? "true" : "false") << reset \
            << " expected: " << color << std::setw(6) << ((expected) ? "true" : "false") << reset \
            << " time: " \
            << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us" \
            << std::endl; \
} while(false);

void basic_test();
void boundary_test();
void long_string_test();
void backtrace_killer_test();
void mixed_character_test();
void non_matching_long_test();
void deeply_nested_test();
void ambiguous_parsing_test();
void chaotic_mixed_test();
void long_string_complex_test();
void character_class_test();
void extended_alphabet_and_escape_test();
void divisible_test();

constexpr int N = 100000;
constexpr int M = 10000;

#endif