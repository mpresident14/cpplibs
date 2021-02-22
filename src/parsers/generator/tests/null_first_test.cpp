#include "src/parsers/generator/null_first.hpp"
#include "src/parsers/generator/testing/nullable_grammar.hpp"
#include "src/testing/unit_test.hpp"

#include <bitset>

#include <boost/dynamic_bitset.hpp>

using namespace std;
using namespace prez::unit_test;


TEST(GetNullabilities) {
  // NULLABLE(S) = false
  // NULLABLE(C) = true
  // NULLABLE(X) = false
  // NULLABLE(Y) = true
  boost::dynamic_bitset<> expected("1010"s);

  assertEquals(expected, getNullabilities(GRAMMAR_DATA));
}

TEST(GetFirsts) {
  // FIRST(S) = {t, b, a, z}
  // FIRST(C) = {b, a, z}
  // FIRST(X) = {a}
  // FIRST(Y) = {b, z}
  boost::dynamic_bitset<> expectedS("1111"s);
  boost::dynamic_bitset<> expectedC("1110"s);
  boost::dynamic_bitset<> expectedX("0100"s);
  boost::dynamic_bitset<> expectedY("1010"s);

  vector<boost::dynamic_bitset<>> actual = getNullsAndFirsts(GRAMMAR_DATA).second;
  assertEquals(expectedS, actual[S]);
  assertEquals(expectedC, actual[C]);
  assertEquals(expectedX, actual[X]);
  assertEquals(expectedY, actual[Y]);
}

int main() {
  runTests();

  return 0;
}
