#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;


TEST(success_exact_integer) {
  auto p = pcomb::integer();

  pcomb::ParseResult<int> result = p->tryParse("123");
  assertParseResult(result, 123, "");
}

TEST(success_exact_hex) {
  auto p = pcomb::integer(16);

  pcomb::ParseResult<int> result = p->tryParse("FF");
  assertParseResult(result, 0xFF, "");
}


TEST(success_leftover) {
  auto p = pcomb::integer();

  pcomb::ParseResult<int> result = p->tryParse("123hello");
  assertParseResult(result, 123, "hello");
}

TEST(failure_mismatched) {
  auto p = pcomb::integer(16);

  pcomb::ParseResult<int> result = p->tryParse("hey");
  assertEmptyParseResult(result, "hey");
}


int main() { return runTests(); }
