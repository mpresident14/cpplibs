#include "src/parsers/generator/dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

void testAddAndRun() {
  using DFAIntChar = DFA<int, char>;
  DFAIntChar dfa(1);
  DFAIntChar::Node* node2 = dfa.addTransition(dfa.getRoot(), 'a', 2);
  DFAIntChar::Node* node3 = dfa.addTransition(dfa.getRoot(), 'b', 3);
  DFAIntChar::Node* node4 = dfa.addTransition(node2, 'c', 4);
  DFAIntChar::Node* node5 = dfa.addTransition(node2, 'd', 5);
  // Node value already present, should add transition, but not create a new
  // node
  DFAIntChar::Node* node6 = dfa.addTransition(node3, 'e', 5);
  // Duplicate transition, should ignore.
  DFAIntChar::Node* node7 = dfa.addTransition(node3, 'e', 2);

  TESTER.assertEquals(5, dfa.size());

  TESTER.assertEquals(node2, dfa.run({'a'}));
  TESTER.assertEquals(node3, dfa.run({'b'}));
  TESTER.assertEquals(node4, dfa.run({'a', 'c'}));
  TESTER.assertEquals(node5, dfa.run({'a', 'd'}));
  TESTER.assertEquals(nullptr, dfa.run({'c'}));
  TESTER.assertEquals(nullptr, dfa.run({'a', 'b'}));
  TESTER.assertEquals(node5, dfa.run({'b', 'e'}));

  TESTER.assertEquals(nullptr, node6);
  TESTER.assertEquals(nullptr, node7);
}


int main(int, char**) {
  testAddAndRun();

  return 0;
}
