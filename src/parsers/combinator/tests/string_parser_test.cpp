#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;


TEST(success_exact) {
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse("hello");
  assertParseResult(result, "hello"s, "");
}

TEST(success_leftover) {
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse("hello, there");
  assertParseResult(result, "hello"s, ", there");
}

TEST(failure_mismatched) {
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse("hey");
  assertEmptyParseResult(result, "hey");
}



int main() { return runTests(); }
