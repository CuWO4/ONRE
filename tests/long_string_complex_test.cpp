#include "test_head.h"

void long_string_complex_test() {
  std::cout << "\n=== Long String Complex Pattern Tests ===\n";
  std::string long_mixed_ab;
  for (int i = 0; i < 100000; i++) {
      long_mixed_ab += (i % 2 == 0) ? 'a' : 'b';
  }
  TEST_AND_LOG("(a|b)*", long_mixed_ab, true);
  TEST_AND_LOG("(a*b*)*", long_mixed_ab, true);

  std::string long_as_with_b = std::string(50000, 'a') + "b" + std::string(50000, 'a');
  TEST_AND_LOG("a*ba*", long_as_with_b, true);
  TEST_AND_LOG("a*ca*", long_as_with_b, false);

  std::string long_for_complex = "start" + std::string(100000, 'x') + "end";
  TEST_AND_LOG("start(x)*end", long_for_complex, true);
  TEST_AND_LOG("start(y)*end", long_for_complex, false);

  std::string long_with_optional = "prefix" + std::string(100000, 'x') + "suffix";
  TEST_AND_LOG("prefix(x)+(y)?suffix", long_with_optional, true);
  TEST_AND_LOG("prefix(x)+(y)?suffix", "prefixxsuffix", true);
  TEST_AND_LOG("prefix(x)+(y)?suffix", "prefixsuffix", false);

  std::string long_mixed;
  for (int i = 0; i < 100000; i++) {
      long_mixed += (i % 2 == 0) ? 'a' : 'b';
  }
  TEST_AND_LOG("(a+b)+", long_mixed, true);
  TEST_AND_LOG("(a?b?)+", long_mixed, true);

  std::string long_ab_sequence;
  for (int i = 0; i < 50000; i++) {
      long_ab_sequence += "ab";
  }
  TEST_AND_LOG("(ab)*", long_ab_sequence, true);
  TEST_AND_LOG("(a|b)*", long_ab_sequence, true);
  TEST_AND_LOG("(aa)*", long_ab_sequence, false);
  TEST_AND_LOG(".*", long_ab_sequence, true);
}