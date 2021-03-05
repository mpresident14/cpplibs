#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;


TEST(success) {
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse("hello");
  assertParseResult(result, "hello"s, "");
}


int main() { return runTests(); }
