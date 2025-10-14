#include "test_head.h"

void long_string_complex_test() {
  std::cout << "\n=== Long String Complex Pattern Tests ===\n";
  std::string long_mixed_ab;
  for (int i = 0; i < 100000; i++) {
      long_mixed_ab += (i % 2 == 0) ? 'a' : 'b';
  }
  test_match_and_log<"(a|b)*">(long_mixed_ab, true);
  test_match_and_log<"(a*b*)*">(long_mixed_ab, true);

  std::string long_as_with_b = std::string(50000, 'a') + "b" + std::string(50000, 'a');
  test_match_and_log<"a*ba*">(long_as_with_b, true);
  test_match_and_log<"a*ca*">(long_as_with_b, false);

  std::string long_for_complex = "start" + std::string(100000, 'x') + "end";
  test_match_and_log<"start(x)*end">(long_for_complex, true);
  test_match_and_log<"start(y)*end">(long_for_complex, false);

  std::string long_with_optional = "prefix" + std::string(100000, 'x') + "suffix";
  test_match_and_log<"prefix(x)+(y)?suffix">(long_with_optional, true);
  test_match_and_log<"prefix(x)+(y)?suffix">("prefixxsuffix", true);
  test_match_and_log<"prefix(x)+(y)?suffix">("prefixsuffix", false);

  std::string long_mixed;
  for (int i = 0; i < 100000; i++) {
      long_mixed += (i % 2 == 0) ? 'a' : 'b';
  }
  test_match_and_log<"(a+b)+">(long_mixed, true);
  test_match_and_log<"(a?b?)+">(long_mixed, true);

  std::string long_ab_sequence;
  for (int i = 0; i < 50000; i++) {
      long_ab_sequence += "ab";
  }
  test_match_and_log<"(ab)*">(long_ab_sequence, true);
  test_match_and_log<"(a|b)*">(long_ab_sequence, true);
  test_match_and_log<"(aa)*">(long_ab_sequence, false);
  test_match_and_log<".*">(long_ab_sequence, true);
}