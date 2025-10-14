#include "test_head.h"

#include <cstdlib>

std::string binary_represent(uint64_t x) {
  std::string s;
  while (x) {
    s.append(x % 2 ? "1" : "0");
    x /= 2;
  }
  return std::string { s.rbegin(), s.rend() };
}

#define PATTERN "((((0*1)0)((10*1)0)*((10*1)1)|(0*1)1)((00|1)((10*1)0)*((10*1)1)|01)*((00|1)((10*1)0)*0)|((0*1)0)((10*1)0)*0)(0((00|1)((10*1)0)*((10*1)1)|01)*((00|1)((10*1)0)*0)|1)*(0((00|1)((10*1)0)*((10*1)1)|01)*((00|1)((10*1)0)*))|(((0*1)0)((10*1)0)*((10*1)1)|(0*1)1)((00|1)((10*1)0)*((10*1)1)|01)*((00|1)((10*1)0)*)|((0*1)0)((10*1)0)*"

void divisible_test() {
  std::cout << "\n=== Divisible Test (match all binary number mod 5 remain 2) ===\n";
  test_match_and_log<PATTERN>("000000", false);
  test_match_and_log<PATTERN>("000001", false);
  test_match_and_log<PATTERN>("000010", true);
  test_match_and_log<PATTERN>("000011", false);
  test_match_and_log<PATTERN>("000100", false);
  test_match_and_log<PATTERN>("000101", false);
  test_match_and_log<PATTERN>("000110", false);
  test_match_and_log<PATTERN>("000111", true);
  test_match_and_log<PATTERN>("001000", false);
  test_match_and_log<PATTERN>("001001", false);
  test_match_and_log<PATTERN>("001010", false);
  test_match_and_log<PATTERN>("001011", false);
  test_match_and_log<PATTERN>("001100", true);
  test_match_and_log<PATTERN>("001101", false);
  test_match_and_log<PATTERN>("001110", false);
  test_match_and_log<PATTERN>("001111", false);
  test_match_and_log<PATTERN>("010000", false);
  test_match_and_log<PATTERN>("010001", true);
  test_match_and_log<PATTERN>("010010", false);
  test_match_and_log<PATTERN>("010011", false);
  test_match_and_log<PATTERN>("010100", false);

  for (int _ = 0; _ < 10; _++) {
    uint32_t rand_number = rand();
    test_match_and_log<PATTERN>(binary_represent(rand_number), rand_number % 5 == 2);
  }
}