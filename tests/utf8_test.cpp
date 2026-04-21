#include "test_head.h"

void utf8_test() {
  std::cout << "\n=== UTF-8 Tests ===\n";
  test_match_and_log<u8"你好">((char const*)u8"你好", true);
  test_match_and_log<u8"こんにちは">((char const*)u8"こんにちは", true);
  test_match_and_log<u8"안녕하세요">((char const*)u8"안녕하세요", true);
  test_match_and_log<u8"привет">((char const*)u8"привет", true);
  test_match_and_log<u8"γειά">((char const*)u8"γειά", true);
  test_match_and_log<u8"مرحبا">((char const*)u8"مرحبا", true);
  test_match_and_log<u8"שלום">((char const*)u8"שלום", true);
  test_match_and_log<u8"नमस्ते">((char const*)u8"नमस्ते", true);
  test_match_and_log<u8"😀">((char const*)u8"😀", true);
  test_match_and_log<u8"𐍈">((char const*)u8"𐍈", true);

  test_match_and_log<u8".">((char const*)u8"a", true);
  test_match_and_log<u8".">((char const*)u8"\n", false);
  test_match_and_log<u8"..">((char const*)u8"é", true);
  test_match_and_log<u8"a..b">((char const*)u8"aéb", true);
  test_match_and_log<u8"a.b">((char const*)u8"aéb", false);
  test_match_and_log<u8"é+">((char const*)u8"ééé", true);
  test_match_and_log<u8"(é|è)">((char const*)u8"è", true);
  test_match_and_log<u8"(\xC3\xA9)+">((char const*)u8"éé", true);
  test_match_and_log<u8"(\xF0\x9F\x98\x80)+">((char const*)u8"😀😀", true);
  test_match_and_log<u8"\xE4\xB8\xAD">((char const*)u8"中", true);

  test_replace_and_log<u8"(你好)">("$1", (char const*)u8"你好", (char const*)u8"你好");
  test_replace_and_log<u8"(é)(中)">("$2-$1", (char const*)u8"é中", (char const*)u8"中-é");
  test_replace_and_log<"(😀)(世界)">("$2/$1", (char const*)u8"😀世界", (char const*)u8"世界/😀");
  test_replace_and_log<u8"(こんにちは)">("[$1]", (char const*)u8"こんにちは", (char const*)u8"[こんにちは]");
}
