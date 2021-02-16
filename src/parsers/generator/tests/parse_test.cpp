#include "src/misc/ostreamable.hpp"
#include "src/parsers/generator/testing/expr.hpp"
#include "src/parsers/generator/testing/test_parser.hpp"

#include <memory>
#include <string>
#include <vector>

#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;


ostringstream errBuffer;

using ExprPtr = unique_ptr<Expr>;

/* See test/write_lexer.cpp */
TEST(Parse) {
  ExprPtr e0(test_parser::parseString("3+12 + 4"));
  ExprPtr e1(test_parser::parseString("3+12 *4"));
  ExprPtr e2(test_parser::parseString("3 *12 + 4"));
  ExprPtr e3(test_parser::parseString("3* 12*  4"));

  assertEquals(19, e0->eval());
  assertEquals(51, e1->eval());
  assertEquals(40, e2->eval());
  assertEquals(144, e3->eval());
}

TEST(Parse_invalidTokens) {
  ostringstream expectedErr0;
  expectedErr0 << "Lexer error on line 1 at: a * 24\n"
               << "Previous tokens were: "
               << prez::misc::OStreamable(vector<string>{"INT", "PLUS"});

  string err0 = assertThrows([]() { test_parser::parseString("1 + a * 24"); });
  assertEquals(expectedErr0.str(), err0);
}

TEST(Parse_noParse) {
  ostringstream expectedErr0;
  expectedErr0 << "Parse error on line 1:\n\tStack: "
               << prez::misc::OStreamable(vector<string>{"Expr", "PLUS", "Expr", "STAR", "PLUS"})
               << "\n\tRemaining tokens: " << prez::misc::OStreamable(vector<string>{"INT"});

  string err0 = assertThrows([]() { test_parser::parseString("123 + 24* + 5"); });
  assertEquals(expectedErr0.str(), err0);

  // Note that INT is not in lookahead set after Expr STAR INT, so INT doesn't
  // reduce to Expr
  ostringstream expectedErr1;
  expectedErr1 << "Parse error on line 2:\n\tStack: "
               << prez::misc::OStreamable(vector<string>{"Expr", "STAR", "INT", "INT"})
               << "\n\tRemaining tokens: "
               << prez::misc::OStreamable(vector<string>{"STAR", "PLUS", "INT"});

  string err1 = assertThrows([]() { test_parser::parseString("3 * 2\n 34* + 5"); });
  assertEquals(expectedErr1.str(), err1);
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  runTests();

  return 0;
}
