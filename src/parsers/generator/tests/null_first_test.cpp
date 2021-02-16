#include "src/parsers/generator/null_first.hpp"

#include "src/parsers/generator/testing/nullable_grammar.hpp"

#include <bitset>

#include <boost/dynamic_bitset.hpp>
#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

void testGetNullabilities() {
  // NULLABLE(S) = false
  // NULLABLE(C) = true
  // NULLABLE(X) = false
  // NULLABLE(Y) = true
  boost::dynamic_bitset<> expected("1010"s);

  TESTER.assertEquals(expected, getNullabilities(GRAMMAR_DATA));
}

void testGetFirsts() {
  // FIRST(S) = {t, b, a, z}
  // FIRST(C) = {b, a, z}
  // FIRST(X) = {a}
  // FIRST(Y) = {b, z}
  boost::dynamic_bitset<> expectedS("1111"s);
  boost::dynamic_bitset<> expectedC("1110"s);
  boost::dynamic_bitset<> expectedX("0100"s);
  boost::dynamic_bitset<> expectedY("1010"s);

  vector<boost::dynamic_bitset<>> actual =
      getNullsAndFirsts(GRAMMAR_DATA).second;
  TESTER.assertEquals(expectedS, actual[S]);
  TESTER.assertEquals(expectedC, actual[C]);
  TESTER.assertEquals(expectedX, actual[X]);
  TESTER.assertEquals(expectedY, actual[Y]);
}

int main() {
  testGetNullabilities();
  testGetFirsts();

  return 0;
}
