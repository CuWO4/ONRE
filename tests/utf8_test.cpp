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
  test_match_and_log<u8".">((char const*)u8"é", true);
  test_match_and_log<u8".">((char const*)u8"中", true);
  test_match_and_log<u8".">((char const*)u8"😀", true);
  test_match_and_log<u8"..">((char const*)u8"😀", false);
  test_match_and_log<u8"a.b">((char const*)u8"aéb", true);
  test_match_and_log<u8"a.b">((char const*)u8"a😀b", true);
  test_match_and_log<u8"a.b">((char const*)u8"a😀😀b", false);
  test_match_and_log<u8"é+">((char const*)u8"ééé", true);
  test_match_and_log<u8"(é|è)">((char const*)u8"è", true);
  test_match_and_log<u8"(\xC3\xA9)+">((char const*)u8"éé", true);
  test_match_and_log<u8"(\xF0\x9F\x98\x80)+">((char const*)u8"😀😀", true);
  test_match_and_log<u8"\xE4\xB8\xAD">((char const*)u8"中", true);
  test_match_and_log<u8"(a|😀)+b">((char const*)u8"a😀a😀b", true);
  test_match_and_log<u8"((你好|hello)*世界)">((char const*)u8"hello你好hello世界", true);
  test_match_and_log<u8"(ab|a😀)*c">((char const*)u8"a😀abc", true);
  test_match_and_log<u8"(😀|中|[a-z])+!">((char const*)u8"😀中abc!", true);
  test_match_and_log<u8"(hello|你好)(世界|world)">((char const*)u8"你好世界", true);
  test_match_and_log<u8"\\u{A}">("\n", true);
  test_match_and_log<u8"\\u{41}">("A", true);
  test_match_and_log<u8"\\u{4F60}">((char const*)u8"你", true);
  test_match_and_log<u8"\\u{4F60}\\u{597D}">((char const*)u8"你好", true);
  test_match_and_log<u8"[\\u{4F60}-\\u{4F60}]">((char const*)u8"你", true);
  // test_match_and_log<u8"[^\\u{4F60}]">((char const*)u8"你", false);
  test_match_and_log<u8"\\u{10AAAA}">((char const*)u8"\U0010AAAA", true);
  test_match_and_log<u8"[一-上]">((char const*)u8"一", true);
  test_match_and_log<u8"[一-上]">((char const*)u8"上", true);
  test_match_and_log<u8"[一-上]">((char const*)u8"丁", true);
  test_match_and_log<u8"[一-上]">((char const*)u8"字", false);
  test_match_and_log<"[^abc]">("a", false);
  test_match_and_log<"[^abc]">("d", true);
  test_match_and_log<"[^abc]">((char const*)u8"é", true);
  test_match_and_log<"[^abc]">((char const*)u8"字", true);
  test_match_and_log<"[^abc]">((char const*)u8"😀", true);
  // test_match_and_log<u8"[^一-上]">((char const*)u8"一", false);
  // test_match_and_log<u8"[^一-上]">((char const*)u8"上", false);
  // test_match_and_log<u8"[^一-上]">((char const*)u8"丁", false);
  // test_match_and_log<u8"[^一-上]">((char const*)u8"字", true);
  test_match_and_log<u8"([a-z]|[一-上]|[0-9])+😀*">((char const*)u8"abc一上丁123😀😀", true);

  // 😁 = \xF0\x9F\x98\x81, 🙀 = \xF0\x9F\x99\x80, 😀 = \xF0\x9F\x98\x80
  test_match_and_log<u8"[😁🙀]">((char const*)u8"😁", true);
  test_match_and_log<u8"[😁🙀]">((char const*)u8"🙀", true);
  test_match_and_log<u8"[😁🙀]">((char const*)u8"😀", false);
  // test_match_and_log<u8"[^😁🙀]">((char const*)u8"😁", false);
  // test_match_and_log<u8"[^😁🙀]">((char const*)u8"🙀", false);
  // test_match_and_log<u8"[^😁🙀]">((char const*)u8"😀", true);

  // ¢ = \xC2\xA2, é = \xC3\xA9, â = \xC3\xA2
  test_match_and_log<u8"[¢é]">((char const*)u8"¢", true);
  test_match_and_log<u8"[¢é]">((char const*)u8"é", true);
  test_match_and_log<u8"[¢é]">((char const*)u8"â", false);
  // test_match_and_log<u8"[^¢é]">((char const*)u8"¢", false);
  // test_match_and_log<u8"[^¢é]">((char const*)u8"é", false);
  // test_match_and_log<u8"[^¢é]">((char const*)u8"â", true);

  // ¢ = \xC2\xA2, 你 = \xE4\xBD\xA0, 䢠 = \xE4\xA2\xA0
  test_match_and_log<u8"[¢你]">((char const*)u8"¢", true);
  test_match_and_log<u8"[¢你]">((char const*)u8"你", true);
  test_match_and_log<u8"[¢你]">((char const*)u8"䢠", false);
  // test_match_and_log<u8"[^¢你]">((char const*)u8"¢", false);
  // test_match_and_log<u8"[^¢你]">((char const*)u8"你", false);
  // test_match_and_log<u8"[^¢你]">((char const*)u8"䢠", true);

  // ¢ = \xC2\xA2, 😀 = \xF0\x9F\x98\x80, 𢘀 = \xF0\xA2\x98\x80
  test_match_and_log<u8"[¢😀]">((char const*)u8"¢", true);
  test_match_and_log<u8"[¢😀]">((char const*)u8"😀", true);
  test_match_and_log<u8"[¢😀]">((char const*)u8"𢘀", false);
  // test_match_and_log<u8"[^¢😀]">((char const*)u8"¢", false);
  // test_match_and_log<u8"[^¢😀]">((char const*)u8"😀", false);
  // test_match_and_log<u8"[^¢😀]">((char const*)u8"𢘀", true);

  // 你 = \xE4\xBD\xA0, 世 = \xE4\xB8\x96, 佖 = \xE4\xBD\x96
  test_match_and_log<u8"[你世]">((char const*)u8"你", true);
  test_match_and_log<u8"[你世]">((char const*)u8"世", true);
  test_match_and_log<u8"[你世]">((char const*)u8"佖", false);
  // test_match_and_log<u8"[^你世]">((char const*)u8"你", false);
  // test_match_and_log<u8"[^你世]">((char const*)u8"世", false);
  // test_match_and_log<u8"[^你世]">((char const*)u8"佖", true);

  // 你 = \xE4\xBD\xA0, 😀 = \xF0\x9F\x98\x80, hybrid = \xF0\xBD\xA0\x80
  test_match_and_log<u8"[你😀]">((char const*)u8"你", true);
  test_match_and_log<u8"[你😀]">((char const*)u8"😀", true);
  test_match_and_log<u8"[你😀]">("\xF0\xBD\xA0\x80", false);
  // test_match_and_log<u8"[^你😀]">((char const*)u8"你", false);
  // test_match_and_log<u8"[^你😀]">((char const*)u8"😀", false);
  // test_match_and_log<u8"[^你😀]">("\xF0\xBD\xA0\x80", true);

  const char invalid_utf8_1[] = "\x80";
  const char invalid_utf8_2[] = "\xC0\xAF";
  const char invalid_utf8_3[] = "\xE4\xB8";
  const char invalid_utf8_4[] = "\xF5";
  test_match_and_log<u8".">(invalid_utf8_1, false);
  test_match_and_log<u8".">(invalid_utf8_2, false);
  test_match_and_log<u8".">(invalid_utf8_3, false);
  test_match_and_log<u8".">(invalid_utf8_4, false);
  test_match_and_log<u8".*">(invalid_utf8_1, false);
  test_match_and_log<u8".*">(invalid_utf8_2, false);

  test_replace_and_log<u8"(你好)">("$1", (char const*)u8"你好", (char const*)u8"你好");
  test_replace_and_log<u8"(é)(中)">("$2-$1", (char const*)u8"é中", (char const*)u8"中-é");
  test_replace_and_log<"(😀)(世界)">("$2/$1", (char const*)u8"😀世界", (char const*)u8"世界/😀");
  test_replace_and_log<u8"(こんにちは)">("[$1]", (char const*)u8"こんにちは", (char const*)u8"[こんにちは]");
  test_replace_and_log<u8"(.)">("<$1>", (char const*)u8"😀", (char const*)u8"<😀>");
  test_replace_and_log<u8"a.b">("$0", (char const*)u8"a😀b", (char const*)u8"a😀b");
  test_replace_and_log<u8"(hello|你好)(世界|world)">("$2-$1", (char const*)u8"hello世界", (char const*)u8"世界-hello");
}
