#include "src/parsers/combinator/parsers.hpp"

#include <iostream>

using namespace std;
namespace pc = prez::pcomb;

int main() {
  auto p1 = pc::builder(pc::altShared<string>(pc::str("hey"), pc::str("hello")))
                .withErrCheckpt()
                .withName("Greeting")
                .build();

  auto p2 = pc::seqShared(pc::integer(), p1, pc::integer());

  auto p3 = pc::mapTuple(p2, [](int m, const string& str, int n) { return m + str.size() + n; });


  cout << p3->parse("123hello456", {true}) << endl;
}
