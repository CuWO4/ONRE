#ifndef TEST_HEAD_H_
#define TEST_HEAD_H_

#include "../onre.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <streambuf>

inline bool g_test_verbose = false;
inline size_t g_test_total = 0;
inline size_t g_test_passed = 0;

class NullBuffer : public std::streambuf {
public:
  int overflow(int c) override {
    return traits_type::not_eof(c);
  }
};

inline NullBuffer g_test_null_buffer;
inline std::ostream g_test_null_stream(&g_test_null_buffer);

inline void reset_test_counters() {
  g_test_total = 0;
  g_test_passed = 0;
}

inline void set_test_verbose(bool verbose) {
  g_test_verbose = verbose;
}

inline std::ostream& test_output_stream(bool passed) {
  if (g_test_verbose) return std::cout;
  return passed ? g_test_null_stream : std::cerr;
}

inline void record_test_result(bool passed) {
  ++g_test_total;
  if (passed) ++g_test_passed;
}

inline void print_test_summary() {
  std::cout << g_test_passed << "/" << g_test_total << " tests passed" << std::endl;
}

inline std::string abbr(std::string s, size_t max_len) {
  size_t subpart_len = max_len / 2 - 2;
  return s.length() <= max_len
    ? s
    : s.substr(0, subpart_len) + "...." + s.substr(s.length() - subpart_len, subpart_len);
}

inline void replace_all(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

template<onre::impl::FixedString pattern>
void test_match_and_log(std::string_view str, bool expected) {
  auto start = std::chrono::high_resolution_clock::now();
  bool result = onre::match<pattern>(str);
  auto end = std::chrono::high_resolution_clock::now();
  bool passed = (result == expected);
  record_test_result(passed);
  std::ostream& out = test_output_stream(passed);
  if (!g_test_verbose && passed) return;
  std::string s(str);
  replace_all(s, "\n", "\\n");
  replace_all(s, "\t", "\\t");
  replace_all(s, "\f", "\\f");
  replace_all(s, "\r", "\\r");
  const char* color = passed ? "\033[1;32m" : "\033[1;31m";
  const char* reset = "\033[0m";
  std::string pattern_s(pattern.c_str());
  out << "pattern: " << std::left << std::setw(27) << abbr(pattern_s, 25)
      << " pattern_len: " << std::setw(5) << pattern_s.length()
      << " str: " << std::setw(22) << abbr(s, 20)
      << " str_len: " << std::setw(8) << std::to_string(s.length())
      << " result: " << color << std::setw(6) << (result ? "true" : "false") << reset
      << " expected: " << color << std::setw(6) << (expected ? "true" : "false") << reset
      << " time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us"
      << std::endl;
}

template<onre::impl::FixedString pattern>
void test_replace_and_log(std::string_view rule, std::string_view str, std::string_view expected) {
  auto start = std::chrono::high_resolution_clock::now();
  std::string result = onre::replace<pattern>(rule, str);
  auto end = std::chrono::high_resolution_clock::now();
  std::string s(str), expected_s(expected);
  bool passed = (result == expected_s);
  record_test_result(passed);
  std::ostream& out = test_output_stream(passed);
  if (!g_test_verbose && passed) return;
  replace_all(s, "\n", "\\n");
  replace_all(s, "\t", "\\t");
  replace_all(s, "\f", "\\f");
  replace_all(s, "\r", "\\r");
  replace_all(result, "\n", "\\n");
  replace_all(result, "\t", "\\t");
  replace_all(result, "\f", "\\f");
  replace_all(result, "\r", "\\r");
  replace_all(expected_s, "\n", "\\n");
  replace_all(expected_s, "\t", "\\t");
  replace_all(expected_s, "\f", "\\f");
  replace_all(expected_s, "\r", "\\r");
  const char* color = passed ? "\033[1;32m" : "\033[1;31m";
  const char* reset = "\033[0m";
  std::string pattern_s(pattern.c_str());
  out << "pattern: " << std::left << std::setw(20) << abbr(pattern_s, 18)
      << " pattern_len: " << std::setw(5) << pattern_s.length()
      << " replace_rule: " << std::setw(20) << abbr(std::string(rule), 18)
      << " str: " << std::setw(20) << abbr(s, 18)
      << " str_len: " << std::setw(8) << std::to_string(s.length())
      << " result: " << color << std::setw(20) << abbr(result, 18) << reset
      << " expected: " << color << std::setw(20) << abbr(expected_s, 18) << reset
      << " time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us"
      << std::endl;
}

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
void quantifier_test();
void divisible_test();
void utf8_test();
void notdotall_test();
void dotall_test();

void replace_basic_test();
void replace_boundary_test();
void replace_long_string_test();
void replace_backtrace_killer_test();
void replace_deeply_nested_test();
void replace_ambiguous_parsing_test();
void replace_chaotic_mixed_test();
void replace_long_string_complex_test();
void replace_character_class_test();
void replace_extended_alphabet_and_escape_test();
void replace_quantifier_test();
void replace_longest_match_test();
void replace_non_capturing_group_test();

constexpr int N = 100000;
constexpr int M = 10000;

#endif