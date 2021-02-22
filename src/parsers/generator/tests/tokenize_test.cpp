#include "src/parsers/generator/testing/test_lexer.hpp"
#include "src/testing/unit_test.hpp"

#include <string>
#include <vector>

using namespace std;
using namespace prez::unit_test;
using namespace test_lexer;


constexpr static int DNOTABC = -1;
constexpr static int FOR = -2;
constexpr static int HabLLO = -3;
constexpr static int HDOTLLO = -4;
constexpr static int H_LLO = -5;
constexpr static int STRLIT = -6;
constexpr static int NUMBER = -7;
constexpr static int IDENT = -8;

TEST(GeneralTokenization) {
  vector<StackObj> tokens = tokenize(R"( "Hello" "Hi \"Bob\"" 123 abc123 de h]llo )");

  StackObj t0 = move(tokens[0]);
  assertEquals(STRLIT, t0.getSymbol());
  if (t0.getSymbol() == STRLIT) {
    assertEquals("Hello", *static_cast<string*>(t0.getObj()));
  }

  StackObj t1 = move(tokens[1]);
  assertEquals(STRLIT, t1.getSymbol());
  if (t1.getSymbol() == STRLIT) {
    assertEquals(R"(Hi \"Bob\")", *static_cast<string*>(t1.getObj()));
  }

  StackObj t2 = move(tokens[2]);
  assertEquals(NUMBER, t2.getSymbol());
  if (t2.getSymbol() == NUMBER) {
    assertEquals(123, *(int*)t2.getObj());
  }

  StackObj t3 = move(tokens[3]);
  assertEquals(IDENT, t3.getSymbol());
  if (t3.getSymbol() == IDENT) {
    assertEquals("abc123", *static_cast<string*>(t3.getObj()));
  }

  StackObj t4 = move(tokens[4]);
  assertEquals(DNOTABC, t4.getSymbol());
  if (t4.getSymbol() == DNOTABC) {
    assertEquals('e', **(char**)t4.getObj());
  }

  assertEquals(HabLLO, tokens[5].getSymbol());
}

TEST(FirstComeFirstServed) {
  vector<StackObj> tokens = tokenize("for h.llo hello hullo hollow");

  assertEquals(FOR, tokens[0].getSymbol());
  assertEquals(HDOTLLO, tokens[1].getSymbol());
  assertEquals(H_LLO, tokens[2].getSymbol());
  assertEquals(H_LLO, tokens[3].getSymbol());
  StackObj t4 = move(tokens[4]);
  assertEquals(IDENT, t4.getSymbol());
  if (t4.getSymbol() == IDENT) {
    assertEquals("hollow", *static_cast<string*>(t4.getObj()));
  }
}

TEST(MatchLongest) {
  vector<StackObj> tokens = tokenize("forbid digit");

  StackObj t0 = move(tokens[0]);
  assertEquals(IDENT, t0.getSymbol());
  if (t0.getSymbol() == IDENT) {
    assertEquals("forbid", *static_cast<string*>(t0.getObj()));
  }
  StackObj t1 = move(tokens[1]);
  assertEquals(IDENT, t1.getSymbol());
  if (t1.getSymbol() == IDENT) {
    assertEquals("digit", *static_cast<string*>(t1.getObj()));
  }
}

TEST(NoLeaksOnError) {
  string err = assertThrows([]() { tokenize("dg {}"); });
  assertNotEqual(string::npos, err.find("NOTABC"));
}

int main() {
  runTests();

  return 0;
}
