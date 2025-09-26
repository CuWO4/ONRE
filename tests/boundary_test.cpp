#include "test_head.h"

void boundary_test() {
  std::cout << "\n=== Boundary Tests ===\n";
  TEST_AND_LOG("", "", true);
  TEST_AND_LOG("", "a", false);
  TEST_AND_LOG("a", "", false);
  TEST_AND_LOG("(a|)", "", true);
  TEST_AND_LOG("(a|)", "a", true);
  TEST_AND_LOG("[a]*", "", true);
  TEST_AND_LOG("[a]", "", false);
}