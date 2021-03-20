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
  assertEquals(nullptr, result.executionLog);
}

TEST(success_leftover_verbose) {
  const char input[] = "hello, there";
  const char expectedRest[] = ", there";
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse(input, {true});

  assertParseResult(result, "hello"s, expectedRest);
  verifyExecLog(result, true, "hello", input, expectedRest, 0);
}

TEST(failure_mismatched) {
  auto p = pcomb::str("hello");

  pcomb::ParseResult<string> result = p->tryParse("hey");

  assertEmptyParseResult(result, "");
  assertEquals(nullptr, result.executionLog);
}

TEST(failure_mismatched_withErrCheckpt_verbose) {
  const char input[] = "hey";
  const char name[] = "CustomName";
  auto p = pcomb::builder(pcomb::str("hello")).withErrCheckpt().withName(name).build();

  pcomb::ParseResult<string> result = p->tryParse("hey", {true});

  assertEmptyParseResult(result, input, name);
  verifyExecLog(result, false, name, input, input, 0);
}


int main() { return runTests(); }
