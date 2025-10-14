#include "test_head.h"

void replace_chaotic_mixed_test() {
  std::cout << "\n=== Replace Chaotic Mixed Tests ===\n";
  test_replace_and_log<"([a-z]+)([0-9]+)([A-Z]+)">("$3:$2:$1", "foo123BAR", "BAR:123:foo");
  test_replace_and_log<"(ab|cd|ef)gh">("<$0>", "efgh", "<efgh>");
  test_replace_and_log<"(a|b|c)+(d|e|f)">("$0->$1", "aaabf", "aaabf->b");
  test_replace_and_log<"([xyz]+)-([0-9]+)">("$2|$1", "xxy-77", "77|xxy");
  test_replace_and_log<"(cat|car)(s?)">("$1|$2", "cars", "car|s");
  test_replace_and_log<"(foo|foobar)(bar)">( "$1-$2", "foobarbar", "foobar-bar");
  test_replace_and_log<"([+\\-]?)([0-9]+)">( "SIGN:$1 VAL:$2", "-42", "SIGN:- VAL:42");
  test_replace_and_log<"(hi)(there)">( "$2,$1", "hithere", "there,hi");
}