#include "src/parsers/generator/build_parser.hpp"

#include "src/parsers/generator/dfa.hpp"
#include "src/parsers/generator/rules.hpp"
#include "src/parsers/generator/testing/expr_grammar.hpp"
#include "src/parsers/generator/utils.hpp"

#include <string>

#include <boost/dynamic_bitset.hpp>
#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;
using namespace test::expr_grammar;
using namespace std::string_literals;

UnitTest TESTER = UnitTest::createTester();

void testNumStates(const DFA<DFARuleSet, int, DFARuleSetHash> &dfa) {
  TESTER.assertEquals(7, dfa.size());
}

void testInitState(const DFA<DFARuleSet, int, DFARuleSetHash> &dfa) {
  /* lookahead = {INT, PLUS, STAR, whitespace} */

  const DFARuleSet &initRules = dfa.getRoot()->getValue();
  const DFARule expected0 = {
      SCONC, {EXPR}, 0, boost::dynamic_bitset<>("0000"s)};
  const DFARule expected1 = {EINT, {INT}, 0, boost::dynamic_bitset<>("0110"s)};
  const DFARule expected2 = {
      EPLUS, {EXPR, PLUS, EXPR}, 0, boost::dynamic_bitset<>("0110"s)};
  const DFARule expected3 = {
      ETIMES, {EXPR, STAR, EXPR}, 0, boost::dynamic_bitset<>("0110"s)};

  TESTER.assertEquals(4, initRules.size());

  // Not using contains() because DFARuleSet uses a special equality operator.
  // We want to use the == operator so that we also compare the lookahead set.

  auto iter0 = initRules.find(expected0);
  if (iter0 == initRules.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expected0, *iter0);
  }

  auto iter1 = initRules.find(expected1);
  if (iter1 == initRules.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expected1, *iter1);
  }

  auto iter2 = initRules.find(expected2);
  if (iter2 == initRules.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expected2, *iter2);
  }

  auto iter3 = initRules.find(expected3);
  if (iter3 == initRules.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expected3, *iter3);
  }
}

void testTransition(const DFA<DFARuleSet, int, DFARuleSetHash> &dfa) {
  const auto &transitions = dfa.getRoot()->getTransitions();
  const DFARuleSet &ruleSetInt = transitions.at(INT)->getValue();
  const DFARuleSet &ruleSetExpr = transitions.at(EXPR)->getValue();
  const DFARule expectedInt = {
      EINT, {INT}, 1, boost::dynamic_bitset<>("0110"s)};
  const DFARule expectedExpr0 = {
      EPLUS, {EXPR, PLUS, EXPR}, 1, boost::dynamic_bitset<>("0110"s)};
  const DFARule expectedExpr1 = {
      ETIMES, {EXPR, STAR, EXPR}, 1, boost::dynamic_bitset<>("0110"s)};
  const DFARule expectedExpr2 = {
      SCONC, {EXPR}, 1, boost::dynamic_bitset<>("0000"s)};

  TESTER.assertEquals(2, transitions.size());

  TESTER.assertEquals(1, ruleSetInt.size());

  auto iter0 = ruleSetInt.find(expectedInt);
  if (iter0 == ruleSetInt.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expectedInt, *iter0);
  }

  TESTER.assertEquals(3, ruleSetExpr.size());

  auto iter1 = ruleSetExpr.find(expectedExpr0);
  if (iter1 == ruleSetExpr.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expectedExpr0, *iter1);
  }

  auto iter2 = ruleSetExpr.find(expectedExpr1);
  if (iter2 == ruleSetExpr.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expectedExpr1, *iter2);
  }

  auto iter3 = ruleSetExpr.find(expectedExpr2);
  if (iter3 == ruleSetExpr.end()) {
    TESTER.assertTrue(false);
  } else {
    TESTER.assertEquals(expectedExpr2, *iter3);
  }
}

int main() {
  auto dfa = buildParserDFA(GRAMMAR_DATA, {});

  testNumStates(dfa);
  testInitState(dfa);
  testTransition(dfa);

  return 0;
}
