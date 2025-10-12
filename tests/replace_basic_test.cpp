#include "test_head.h"

void replace_basic_test() {
  std::cout << "\n=== Replace Basic Tests ===\n";

  onre::Replace<"((ab)*|c|(d)*)|(e)">::eval("", "");
}
