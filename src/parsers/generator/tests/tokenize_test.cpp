#include "src/parsers/generator/testing/test_lexer.hpp"

#include <string>
#include <vector>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;
using namespace test_lexer;

UnitTest TESTER = UnitTest::createTester();

constexpr static int DNOTABC = -1;
constexpr static int FOR = -2;
constexpr static int HabLLO = -3;
constexpr static int HDOTLLO = -4;
constexpr static int H_LLO = -5;
constexpr static int STRLIT = -6;
constexpr static int NUMBER = -7;
constexpr static int IDENT = -8;

void testGeneralTokenization() {
  vector<StackObj> tokens =
      tokenize(R"( "Hello" "Hi \"Bob\"" 123 abc123 de h]llo )");

  StackObj t0 = move(tokens[0]);
  TESTER.assertEquals(STRLIT, t0.getSymbol());
  if (t0.getSymbol() == STRLIT) {
    TESTER.assertEquals("Hello", *static_cast<string *>(t0.getObj()));
  }

  StackObj t1 = move(tokens[1]);
  TESTER.assertEquals(STRLIT, t1.getSymbol());
  if (t1.getSymbol() == STRLIT) {
    TESTER.assertEquals(R"(Hi \"Bob\")", *static_cast<string *>(t1.getObj()));
  }

  StackObj t2 = move(tokens[2]);
  TESTER.assertEquals(NUMBER, t2.getSymbol());
  if (t2.getSymbol() == NUMBER) {
    TESTER.assertEquals(123, *(int *)t2.getObj());
  }

  StackObj t3 = move(tokens[3]);
  TESTER.assertEquals(IDENT, t3.getSymbol());
  if (t3.getSymbol() == IDENT) {
    TESTER.assertEquals("abc123", *static_cast<string *>(t3.getObj()));
  }

  StackObj t4 = move(tokens[4]);
  TESTER.assertEquals(DNOTABC, t4.getSymbol());
  if (t4.getSymbol() == DNOTABC) {
    TESTER.assertEquals('e', **(char **)t4.getObj());
  }

  TESTER.assertEquals(HabLLO, tokens[5].getSymbol());
}

void testFirstComeFirstServed() {
  vector<StackObj> tokens = tokenize("for h.llo hello hullo hollow");

  TESTER.assertEquals(FOR, tokens[0].getSymbol());
  TESTER.assertEquals(HDOTLLO, tokens[1].getSymbol());
  TESTER.assertEquals(H_LLO, tokens[2].getSymbol());
  TESTER.assertEquals(H_LLO, tokens[3].getSymbol());
  StackObj t4 = move(tokens[4]);
  TESTER.assertEquals(IDENT, t4.getSymbol());
  if (t4.getSymbol() == IDENT) {
    TESTER.assertEquals("hollow", *static_cast<string *>(t4.getObj()));
  }
}

void testMatchLongest() {
  vector<StackObj> tokens = tokenize("forbid digit");

  StackObj t0 = move(tokens[0]);
  TESTER.assertEquals(IDENT, t0.getSymbol());
  if (t0.getSymbol() == IDENT) {
    TESTER.assertEquals("forbid", *static_cast<string *>(t0.getObj()));
  }
  StackObj t1 = move(tokens[1]);
  TESTER.assertEquals(IDENT, t1.getSymbol());
  if (t1.getSymbol() == IDENT) {
    TESTER.assertEquals("digit", *static_cast<string *>(t1.getObj()));
  }
}

void testNoLeaksOnError() {
  string err = TESTER.assertThrows([]() { tokenize("dg {}"); });
  TESTER.assertNotEqual(string::npos, err.find("NOTABC"));
}

int main() {
  testGeneralTokenization();
  testFirstComeFirstServed();
  testMatchLongest();
  testNoLeaksOnError();

  return 0;
}
