#include "src/parsers/generator/dfa.hpp"
#include "src/testing/unit_test.hpp"

#include <cstddef>
#include <iostream>
#include <string>

using namespace std;
using namespace prez::unit_test;


TEST(AddAndRun) {
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

  assertEquals(size_t(5), dfa.size());

  assertEquals(node2, dfa.run({'a'}));
  assertEquals(node3, dfa.run({'b'}));
  assertEquals(node4, dfa.run({'a', 'c'}));
  assertEquals(node5, dfa.run({'a', 'd'}));
  assertEquals(nullptr, dfa.run({'c'}));
  assertEquals(nullptr, dfa.run({'a', 'b'}));
  assertEquals(node5, dfa.run({'b', 'e'}));

  assertEquals(nullptr, node6);
  assertEquals(nullptr, node7);
}


int main(int, char**) {
  runTests();

  return 0;
}
