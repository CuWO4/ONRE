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
  TEST_AND_LOG(PATTERN, "000000", false);
  TEST_AND_LOG(PATTERN, "000001", false);
  TEST_AND_LOG(PATTERN, "000010", true);
  TEST_AND_LOG(PATTERN, "000011", false);
  TEST_AND_LOG(PATTERN, "000100", false);
  TEST_AND_LOG(PATTERN, "000101", false);
  TEST_AND_LOG(PATTERN, "000110", false);
  TEST_AND_LOG(PATTERN, "000111", true);
  TEST_AND_LOG(PATTERN, "001000", false);
  TEST_AND_LOG(PATTERN, "001001", false);
  TEST_AND_LOG(PATTERN, "001010", false);
  TEST_AND_LOG(PATTERN, "001011", false);
  TEST_AND_LOG(PATTERN, "001100", true);
  TEST_AND_LOG(PATTERN, "001101", false);
  TEST_AND_LOG(PATTERN, "001110", false);
  TEST_AND_LOG(PATTERN, "001111", false);
  TEST_AND_LOG(PATTERN, "010000", false);
  TEST_AND_LOG(PATTERN, "010001", true);
  TEST_AND_LOG(PATTERN, "010010", false);
  TEST_AND_LOG(PATTERN, "010011", false);
  TEST_AND_LOG(PATTERN, "010100", false);

  for (int _ = 0; _ < 10; _++) {
    uint32_t rand_number = rand();
    TEST_AND_LOG(PATTERN, binary_represent(rand_number), rand_number % 5 == 2);
  }
}