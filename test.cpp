#include "regex.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

#define TEST_AND_LOG(pattern, str, expected) \
do { \
  auto start = std::chrono::high_resolution_clock::now(); \
  bool result = onre::Regex<pattern>::match(str); \
  auto end = std::chrono::high_resolution_clock::now(); \
  std::string s(str); \
  const char* color = (result == expected) ? "\033[1;32m" : "\033[1;31m"; \
  const char* reset = "\033[0m"; \
  std::cout << "pattern: " << std::left << std::setw(25) << pattern \
            << "str: " << std::setw(25) << (s.length() > 20 ? s.substr(0, 20) + "..." : s) \
            << "str_len: " << std::setw(8) << std::to_string(s.length()) \
            << "result: " << color << std::setw(6) << (result ? "true" : "false") << reset \
            << "expected: " << color << std::setw(6) << (expected ? "true" : "false") << reset \
            << "time: " \
            << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us" \
            << std::endl; \
} while(false);

int main() {
  TEST_AND_LOG("a", "a", true);
  TEST_AND_LOG("a", "b", false);
  TEST_AND_LOG("a|b", "a", true);
  TEST_AND_LOG("a|b", "b", true);
  TEST_AND_LOG("a|b", "c", false);
  TEST_AND_LOG("a*", "", true);
  TEST_AND_LOG("a*", "a", true);
  TEST_AND_LOG("a*", "aa", true);
  TEST_AND_LOG("a*", "b", false);
  TEST_AND_LOG("ab", "ab", true);
  TEST_AND_LOG("ab", "a", false);
  TEST_AND_LOG("(ab)*", "abab", true);
  TEST_AND_LOG("(ab)*", "aba", false);
  TEST_AND_LOG("a*b", "aaab", true);
  TEST_AND_LOG("a*b", "b", true);
  TEST_AND_LOG("a*b", "aaac", false);
  TEST_AND_LOG("(a|b)*|c", "abbac", false);
  TEST_AND_LOG("(a|b)*|c", "c", true);
  TEST_AND_LOG("(a|b)*|c", "d", false);

  std::cout << "\n=== Boundary Tests ===\n";
  TEST_AND_LOG("", "", true);
  TEST_AND_LOG("", "a", false);
  TEST_AND_LOG("a", "", false);
  TEST_AND_LOG("a*", "", true);
  TEST_AND_LOG("(a|)", "a", true);
  TEST_AND_LOG("(a|)", "", true);

  std::cout << "\n=== Long String Tests (O(n) performance) ===\n";
  const int N = 100000;
  const int M = 10000;

  std::string long_a(N, 'a');
  TEST_AND_LOG("a*", long_a, true);

  std::string long_ab(N, 'a');
  long_ab += 'b';
  TEST_AND_LOG("a*b", long_ab, true);

  std::string long_alternating;
  for (int i = 0; i < N/2; i++) {
    long_alternating += "ab";
  }
  TEST_AND_LOG("(ab)*", long_alternating, true);

  std::cout << "\n=== Backtracking Killer Tests ===\n";

  std::string no_c(M, 'a');
  TEST_AND_LOG("(a|b)*c", no_c, false);

  std::string almost_two_a(M, 'b');
  almost_two_a += 'a';
  TEST_AND_LOG("(a|b)*a(a|b)*a", almost_two_a, false);

  std::string long_a_simple(M, 'a');
  TEST_AND_LOG("(a*)*", long_a_simple, true);

  std::string long_b(M, 'b');
  TEST_AND_LOG("(a*(b*)*)*", long_b, true);

  std::string prefix(M, 'a');
  std::string suffix(M, 'b');
  TEST_AND_LOG("(a|b)*a(a|b)*b", prefix + "c" + suffix, false);

  std::cout << "\n=== Mixed Character Tests ===\n";
  std::string mixed;
  for (int i = 0; i < M; i++) {
    mixed += ('a' + (i % 26));
  }
  TEST_AND_LOG("(a|b|c|d|e)*", mixed, false);

  std::cout << "\n=== Non-matching Long String Tests ===\n";
  std::string long_numbers(M, '1');
  TEST_AND_LOG("(a|b|c|d|e)*", long_numbers, false);

  std::string long_a_with_b(M, 'a');
  long_a_with_b[M/2] = 'b';
  TEST_AND_LOG("a*", long_a_with_b, false);

  return 0;
}