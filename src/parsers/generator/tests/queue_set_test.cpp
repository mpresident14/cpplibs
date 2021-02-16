#include "src/parsers/generator/queue_set.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;


TEST(Push) {
  QueueSet<string> q;
  q.push("hello");
  q.push("bye");
  q.push("hey");
  q.push("thanks");
  q.push("bye");
  q.push("what's up");
  q.push("hello");

  assertEquals(size_t(5), q.size());
}

TEST(Pop) {
  QueueSet<int> q;
  q.push(1);
  q.push(2);
  q.push(3);
  q.push(4);
  q.push(2);
  q.push(5);
  q.push(1);

  assertEquals(size_t(5), q.size());
  for (int i = 1; i <= 5; ++i) {
    assertEquals(i, q.front());
    q.pop();
  }
}

int main(int, char**) {
  runTests();

  return 0;
}
