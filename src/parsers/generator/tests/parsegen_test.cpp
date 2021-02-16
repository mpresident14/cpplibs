#include "src/parsers/generator/testing/expr.hpp"
#include "src/parsers/generator/testing/pgen_expr_parser.hpp"

#include <memory>

#include <prez/print_stuff.hpp>
#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;
using namespace pgen_expr_parser;

UnitTest TESTER = UnitTest::createTester();
ostringstream errBuffer;

using ExprPtr = unique_ptr<Expr>;

void checkEval(const string &equation, int result) {
  ExprPtr e(parseString(equation));
  TESTER.assertEquals(result, e->eval());
}

void testParse() {
  checkEval("3+12 + 4", 19);
  checkEval("3+12 *4", 51);
  checkEval("3 *12 + 4", 40);
  checkEval("3* 12*  4", 144);
  checkEval("3 - 12 /4", 0);
  checkEval("3 / 12 -  4", -4);
  checkEval("3 * 12 /  4", 9);
  checkEval("3 + 12 -  4", 11);
  checkEval("3 - 12 +  4", -5);
  checkEval("3 + 12 /  4", 6);
  checkEval("-3 + 12", 9);
  checkEval("12 + -3", 9);
  checkEval("12 + -3/3", 11);
  checkEval("12 + ~-1", 12);
  checkEval("(3+12) * 4", 60);
  checkEval("-(3+12)", -15);
}

void testParse_invalidTokens() {
  ostringstream expectedErr0;
  expectedErr0 << "Lexer error on line 1 at: a * 24\n"
               << "Previous tokens were: " << vector<string>{"INT", "PLUS"};

  string err0 = TESTER.assertThrows([]() { parseString("1 + a * 24"); });
  TESTER.assertEquals(expectedErr0.str(), err0);
  TESTER.assertEquals("INT deleter called\n", errBuffer.str());
  // Clear errBuffer
  errBuffer.str("");
}

void testParse_noParse() {
  ostringstream expectedErr0;
  expectedErr0 << "Parse error on line 1:\n\tStack: "
               << vector<string>{"Expr", "PLUS", "Expr", "STAR", "PLUS"}
               << "\n\tRemaining tokens: " << vector<string>{"INT"};

  string err0 = TESTER.assertThrows([]() { parseString("123 + 24* + 5"); });
  TESTER.assertEquals(expectedErr0.str(), err0);
  TESTER.assertEquals("INT deleter called\n", errBuffer.str());
  // Clear errBuffer
  errBuffer.str("");

  // Note that INT is not in lookahead set after Expr STAR INT, so INT doesn't
  // reduce to Expr
  ostringstream expectedErr1;
  expectedErr1 << "Parse error on line 2:\n\tStack: "
               << vector<string>{"Expr", "STAR", "INT", "INT"}
               << "\n\tRemaining tokens: "
               << vector<string>{"STAR", "PLUS", "INT"};

  string err1 = TESTER.assertThrows([]() { parseString("3 * 2\n 34* + 5"); });
  TESTER.assertEquals(expectedErr1.str(), err1);
  TESTER.assertEquals(
      "INT deleter called\nINT deleter called\nINT deleter called\n",
      errBuffer.str());
  // Clear errBuffer
  errBuffer.str("");
}

int main() {
  // To test output
  cerr.rdbuf(errBuffer.rdbuf());

  testParse();
  testParse_invalidTokens();
  testParse_noParse();

  return 0;
}
