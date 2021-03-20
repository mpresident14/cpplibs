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


TEST(success_leftover_verbose) {
  const char input[] = "123hello";
  const char expectedRest[] = "hello";
  auto p = pcomb::integer();

  pcomb::ParseResult<int> result = p->tryParse(input, {true});

  assertParseResult(result, 123, expectedRest);
  verifyExecLog(result, true, "Integer", input, expectedRest, 0);
}

TEST(failure_mismatched) {
  auto p = pcomb::integer(16);

  pcomb::ParseResult<int> result = p->tryParse("hey");

  assertEmptyParseResult(result, "");
  assertEquals(nullptr, result.executionLog);
}

TEST(failure_mismatched_withErrCheckpt_verbose) {
  const char input[] = "hey";
  const char name[] = "CustomName";

  auto p = pcomb::builder(pcomb::integer(16)).withErrCheckpt().withName(name).build();

  pcomb::ParseResult<int> result = p->tryParse(input, {true});
  assertEmptyParseResult(result, input, name);
  verifyExecLog(result, false, name, input, input, 0);
}


int main() { return runTests(); }
