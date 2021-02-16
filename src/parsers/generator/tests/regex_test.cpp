
#include "src/parsers/generator/regex.hpp"

#include "src/misc/ostreamable.hpp"
#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/regex_merge.hpp"
#include "src/parsers/generator/regex_parser.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using regex_parser::parseString;



ostringstream errBuffer;

TEST(Parse) {
  RgxPtr r0 = RgxPtr(parseString("a"));
  RgxPtr r1 = RgxPtr(parseString("ab*|b"));
  RgxPtr r2 = RgxPtr(parseString("[^a-z]"));
  RgxPtr r3 = RgxPtr(parseString("abcd*"));
  RgxPtr r4 = RgxPtr(parseString("a|b|cd"));
  RgxPtr r5 = RgxPtr(parseString("[1-9]"));
  RgxPtr r6 = RgxPtr(parseString("[1-9]*"));
  RgxPtr r7 = RgxPtr(parseString("\\[^([^\\]]|\\\\.)*\\]"));
  RgxPtr r8 = RgxPtr(parseString("(a|b)+"));
  RgxPtr r9 = RgxPtr(parseString("(ab)?"));

  assertEquals(RgxType::CHARACTER, r0->getType());
  assertEquals(RgxType::ALT, r1->getType());
  assertEquals(RgxType::NOT, r2->getType());
  assertEquals(RgxType::CONCAT, r3->getType());
  assertEquals(RgxType::ALT, r4->getType());
  assertEquals(RgxType::RANGE, r5->getType());
  assertEquals(RgxType::STAR, r6->getType());
  assertEquals(RgxType::CONCAT, r7->getType());
  assertEquals(RgxType::CONCAT, r8->getType());
  assertEquals(RgxType::ALT, r9->getType());

  // No conflicts
  assertEquals("", errBuffer.str());
}

TEST(ParseError) {
  ostringstream expectedErr0;
  expectedErr0 << "Parse error on line 1:\n\tStack: "
               << prez::misc::OStreamable(vector<string>{"Concats", "LPAREN", "Regex"})
               << "\n\tRemaining tokens: " << prez::misc::OStreamable(vector<string>{});

  string err0 = assertThrows([]() { parseString("abc(b*"); });
  assertEquals(expectedErr0.str(), err0);

  ostringstream expectedErr1;
  expectedErr1 << "Parse error on line 1:\n\tStack: "
               << prez::misc::OStreamable(vector<string>{"Regex", "BAR", "STAR"})
               << "\n\tRemaining tokens: " << prez::misc::OStreamable(vector<string>{"CHAR"});

  string err1 = assertThrows([]() { parseString("abc|*d"); });
  assertEquals(expectedErr1.str(), err1);

  // No conflicts
  assertEquals("", errBuffer.str());
}

TEST(GetDeriv_character) {
  RgxPtr r1 = RgxPtr(parseString("a"));

  assertEquals(Epsilon(), *r1->getDeriv('a'));
  assertEquals(EmptySet(), *r1->getDeriv('b'));
}

TEST(GetDeriv_alt) {
  RgxPtr r1 = RgxPtr(parseString("ac|ad"));
  RgxPtr r2 = RgxPtr(parseString("ac|bd"));

  RgxPtr e1 = RgxPtr(parseString("c|d"));
  RgxPtr e2 = RgxPtr(parseString("d"));

  assertEquals(*e1, *r1->getDeriv('a'));
  assertEquals(EmptySet(), *r1->getDeriv('b'));
  assertEquals(*e2, *r2->getDeriv('b'));
}

TEST(GetDeriv_concat) {
  RgxPtr r1 = RgxPtr(parseString("ac"));
  RgxPtr r2 = RgxPtr(parseString("a*c"));

  RgxPtr e1 = RgxPtr(parseString("c"));

  assertEquals(*e1, *r1->getDeriv('a'));
  assertEquals(EmptySet(), *r1->getDeriv('b'));
  assertEquals(*r2, *r2->getDeriv('a'));
  assertEquals(Epsilon(), *r2->getDeriv('c'));
}

TEST(GetDeriv_star) {
  RgxPtr r1 = RgxPtr(parseString("a*"));

  assertEquals(*r1, *r1->getDeriv('a'));
  assertEquals(EmptySet(), *r1->getDeriv('b'));
}

TEST(GetDeriv_brackets) {
  RgxPtr r1 = RgxPtr(parseString("[0-9]"));
  RgxPtr r2 = RgxPtr(parseString("[^0-9]"));
  RgxPtr r3 = RgxPtr(parseString("[\\]*$-]"));
  RgxPtr r4 = RgxPtr(parseString("[^\\]*$-]"));

  assertEquals(EmptySet(), *r1->getDeriv('a'));
  assertEquals(Epsilon(), *r1->getDeriv('7'));
  assertEquals(Epsilon(), *r2->getDeriv('a'));
  assertEquals(EmptySet(), *r2->getDeriv('7'));
  assertEquals(Epsilon(), *r3->getDeriv(']'));
  assertEquals(EmptySet(), *r3->getDeriv('a'));
  assertEquals(EmptySet(), *r4->getDeriv(']'));
  assertEquals(Epsilon(), *r4->getDeriv('a'));
}

TEST(HashFn) {
  RgxPtr r1 = RgxPtr(parseString("a"));
  unordered_set<RgxPtr, Regex::PtrHash> rgxs = {r1->getDeriv('b')};
  assertTrue(rgxs.contains(r1->getDeriv('c')));
}

TEST(RgxDFAToCode_withNullableRegex) {
  GrammarData gd = {
      {
          {"", "", NONE, Assoc::NONE, "", "", "a*", 0},
          {"", "", NONE, Assoc::NONE, "", "", "[1-9][0-9]*", 0},
          {"", "", NONE, Assoc::NONE, "", "", "for", 0},
      },
      {},
      {}};

  vector<pair<string, int>> patterns = {{"a*", 1}, {"[1-9][0-9]*", 2}, {"for", 3}};

  ostringstream out;
  out.setstate(ios_base::badbit);
  mergedRgxDFAToCode(out, gd);

  assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}

TEST(RgxDFAToCode_withInvalidRegex) {
  GrammarData gd = {
      {
          {"", "", NONE, Assoc::NONE, "", "", ".", 0},
          {"", "", NONE, Assoc::NONE, "", "", "9)[0-9]*", 0},
          {"", "", NONE, Assoc::NONE, "", "", "for", 0},
      },
      {},
      {}};

  ostringstream out;
  out.setstate(ios_base::badbit);
  assertThrows(([&]() { mergedRgxDFAToCode(out, gd); }));
}

int main(int, char**) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  runTests();

  return 0;
}
