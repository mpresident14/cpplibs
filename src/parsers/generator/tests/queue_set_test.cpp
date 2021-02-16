#include "src/parsers/generator/queue_set.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

void testPush() {
  QueueSet<string> q;
  q.push("hello");
  q.push("bye");
  q.push("hey");
  q.push("thanks");
  q.push("bye");
  q.push("what's up");
  q.push("hello");

  TESTER.assertEquals(5, q.size());
}

void testPop() {
  QueueSet<int> q;
  q.push(1);
  q.push(2);
  q.push(3);
  q.push(4);
  q.push(2);
  q.push(5);
  q.push(1);

  TESTER.assertEquals(5, q.size());
  for (size_t i = 1; i <= 5; ++i) {
    TESTER.assertEquals(i, q.front());
    q.pop();
  }
}

int main(int, char**) {
  testPush();
  testPop();

  return 0;
}
