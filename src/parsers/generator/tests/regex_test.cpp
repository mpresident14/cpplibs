
#include "src/parsers/generator/regex.hpp"

#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/regex_merge.hpp"
#include "src/parsers/generator/regex_parser.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

#include <prez/print_stuff.hpp>
#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;
using regex_parser::parseString;

UnitTest TESTER = UnitTest::createTester();

ostringstream errBuffer;

void testParse() {
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

  TESTER.assertEquals(RgxType::CHARACTER, r0->getType());
  TESTER.assertEquals(RgxType::ALT, r1->getType());
  TESTER.assertEquals(RgxType::NOT, r2->getType());
  TESTER.assertEquals(RgxType::CONCAT, r3->getType());
  TESTER.assertEquals(RgxType::ALT, r4->getType());
  TESTER.assertEquals(RgxType::RANGE, r5->getType());
  TESTER.assertEquals(RgxType::STAR, r6->getType());
  TESTER.assertEquals(RgxType::CONCAT, r7->getType());
  TESTER.assertEquals(RgxType::CONCAT, r8->getType());
  TESTER.assertEquals(RgxType::ALT, r9->getType());

  // No conflicts
  TESTER.assertEquals("", errBuffer.str());
}

void testParseError() {
  ostringstream expectedErr0;
  expectedErr0 << "Parse error on line 1:\n\tStack: "
               << vector<string>{ "Concats", "LPAREN", "Regex" }
               << "\n\tRemaining tokens: " << vector<string>{};

  string err0 = TESTER.assertThrows([]() { parseString("abc(b*"); });
  TESTER.assertEquals(expectedErr0.str(), err0);

  ostringstream expectedErr1;
  expectedErr1 << "Parse error on line 1:\n\tStack: " << vector<string>{ "Regex", "BAR", "STAR" }
               << "\n\tRemaining tokens: " << vector<string>{ "CHAR" };

  string err1 = TESTER.assertThrows([]() { parseString("abc|*d"); });
  TESTER.assertEquals(expectedErr1.str(), err1);

  // No conflicts
  TESTER.assertEquals("", errBuffer.str());
}

void testGetDeriv_character() {
  RgxPtr r1 = RgxPtr(parseString("a"));

  TESTER.assertEquals(Epsilon(), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_alt() {
  RgxPtr r1 = RgxPtr(parseString("ac|ad"));
  RgxPtr r2 = RgxPtr(parseString("ac|bd"));

  RgxPtr e1 = RgxPtr(parseString("c|d"));
  RgxPtr e2 = RgxPtr(parseString("d"));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*e2, *r2->getDeriv('b'));
}

void testGetDeriv_concat() {
  RgxPtr r1 = RgxPtr(parseString("ac"));
  RgxPtr r2 = RgxPtr(parseString("a*c"));

  RgxPtr e1 = RgxPtr(parseString("c"));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*r2, *r2->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('c'));
}

void testGetDeriv_star() {
  RgxPtr r1 = RgxPtr(parseString("a*"));

  TESTER.assertEquals(*r1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_brackets() {
  RgxPtr r1 = RgxPtr(parseString("[0-9]"));
  RgxPtr r2 = RgxPtr(parseString("[^0-9]"));
  RgxPtr r3 = RgxPtr(parseString("[\\]*$-]"));
  RgxPtr r4 = RgxPtr(parseString("[^\\]*$-]"));

  TESTER.assertEquals(EmptySet(), *r1->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r1->getDeriv('7'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r2->getDeriv('7'));
  TESTER.assertEquals(Epsilon(), *r3->getDeriv(']'));
  TESTER.assertEquals(EmptySet(), *r3->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r4->getDeriv(']'));
  TESTER.assertEquals(Epsilon(), *r4->getDeriv('a'));
}

void testHashFn() {
  RgxPtr r1 = RgxPtr(parseString("a"));
  unordered_set<RgxPtr, Regex::PtrHash> rgxs = { r1->getDeriv('b') };
  TESTER.assertTrue(rgxs.contains(r1->getDeriv('c')));
}

void testRgxDFAToCode_withNullableRegex() {
  GrammarData gd = { {
                         { "", "", NONE, Assoc::NONE, "", "", "a*", 0 },
                         { "", "", NONE, Assoc::NONE, "", "", "[1-9][0-9]*", 0 },
                         { "", "", NONE, Assoc::NONE, "", "", "for", 0 },
                     },
                     {},
                     {} };

  vector<pair<string, int>> patterns = { { "a*", 1 }, { "[1-9][0-9]*", 2 }, { "for", 3 } };

  ostringstream out;
  out.setstate(ios_base::badbit);
  mergedRgxDFAToCode(out, gd);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}

void testRgxDFAToCode_withInvalidRegex() {
  GrammarData gd = { {
                         { "", "", NONE, Assoc::NONE, "", "", ".", 0 },
                         { "", "", NONE, Assoc::NONE, "", "", "9)[0-9]*", 0 },
                         { "", "", NONE, Assoc::NONE, "", "", "for", 0 },
                     },
                     {},
                     {} };

  ostringstream out;
  out.setstate(ios_base::badbit);
  TESTER.assertThrows(([&]() { mergedRgxDFAToCode(out, gd); }));
}

int main(int, char**) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  testParse();
  testParseError();
  testGetDeriv_character();
  testGetDeriv_alt();
  testGetDeriv_concat();
  testGetDeriv_star();
  testGetDeriv_brackets();
  testHashFn();
  testRgxDFAToCode_withNullableRegex();
  testRgxDFAToCode_withInvalidRegex();

  return 0;
}
