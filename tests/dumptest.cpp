#include "test_head.h"

void dummy() {
	(void)onre::debug::DumpRE<onre::impl::EmptySet>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Epsilon>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Wildcard>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Wildcard1>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Wildcard2>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Wildcard3>::to_string();

	(void)onre::debug::DumpRE<onre::impl::Or<onre::impl::Char<'a'>, onre::impl::Char<'b'>>>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Concat<onre::impl::Char<'a'>, onre::impl::Or<onre::impl::Char<'b'>, onre::impl::Char<'c'>>>>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Closure<onre::impl::Or<onre::impl::Char<'a'>, onre::impl::Char<'b'>>>>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Seq<onre::impl::Char<'a'>, onre::impl::Set<0>, onre::impl::Omega>>::to_string();
	(void)onre::debug::DumpRE<onre::impl::Except<onre::impl::TypeList<onre::impl::Char<'a'>, onre::impl::Char<'b'>>>>::to_string();

	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"a|b">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"ab">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"(ab)*">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"((a|b)|(c|d))|(e|f)|(g|h)">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"([a-c][0-2])+">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"a(?#middle)(b|c)">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"a(?:b|c)d">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"((a|b)(c|d))*|(e|f)(g|h)*">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"(a|(b|c))*d">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"\\d+\\w*">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"[^abc]">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"[a-zA-Z0-9_]+">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"start(x)*end">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"prefix(x)+(y)?suffix">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"((你好|hello)*世界)">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<u8"\\u{4F60}\\u{597D}">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<u8"[一-上]">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<u8"\\u{10AAAA}">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<u8"(😀|中|[a-z])+!">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<u8"([a-c][0-2])+">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"((a|b)|(c|d))|(e|f)|(g|h)">::type>::to_string();
	(void)onre::debug::DumpRE<typename onre::impl::RegexScan<"(ab|a😀)*c">::type>::to_string();

	(void)onre::debug::DumpDFA<"a|b">::to_string();
	(void)onre::debug::DumpTNFA<"(ab)*">::to_string();
	(void)onre::debug::DumpDFA<u8"\\u{4F60}\\u{597D}">::to_string();
	(void)onre::debug::DumpTNFA<u8"(😀|中|[a-z])+!">::to_string();
	(void)onre::debug::DumpDFA<"((a|b)(c|d))*|(e|f)(g|h)*">::to_string();
	(void)onre::debug::DumpTNFA<"(a|(b|c))*d">::to_string();
	(void)onre::debug::DumpDFA<"\\d+\\w*">::to_string();
	(void)onre::debug::DumpTNFA<"[^abc]">::to_string();
	(void)onre::debug::DumpDFA<"[a-zA-Z0-9_]+">::to_string();
	(void)onre::debug::DumpTNFA<"start(x)*end">::to_string();
	(void)onre::debug::DumpDFA<"prefix(x)+(y)?suffix">::to_string();
	(void)onre::debug::DumpTNFA<"((你好|hello)*世界)">::to_string();
	(void)onre::debug::DumpDFA<u8"[一-上]">::to_string();
	(void)onre::debug::DumpTNFA<u8"\\u{10AAAA}">::to_string();
	(void)onre::debug::DumpDFA<u8"([a-c][0-2])+">::to_string();
	(void)onre::debug::DumpTNFA<"((a|b)|(c|d))|(e|f)|(g|h)">::to_string();
	(void)onre::debug::DumpDFA<"(ab|a😀)*c">::to_string();
}