#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

#include <functional>

using namespace std;
using namespace prez::unit_test;
using namespace prez;

auto P_HELLO = pcomb::strShared("hello");
auto GET_SIZE = std::mem_fun_ref(&std::string::size);

TEST(success_exact) {
  auto p = pcomb::map(P_HELLO, GET_SIZE);

  auto result = p->tryParse("hello");

  assertParseResult(result, 5ul, "");
  assertEquals(nullptr, result.executionLog);
}

// TEST(success_leftover_verbose) {
//   auto p = pcomb::str("hello");

//   pcomb::ParseResult<string> result = p->tryParse("hello, there", {true});

//   assertParseResult(result, "hello"s, ", there");
//   verifyExecLog(result, true, 12, 0);
// }

// TEST(failure_mismatched) {
//   auto p = pcomb::str("hello");

//   pcomb::ParseResult<string> result = p->tryParse("hey");

//   assertEmptyParseResult(result, "");
//   assertEquals(nullptr, result.executionLog);
// }

// TEST(failure_mismatched_withErrCheckpt_verbose) {
//   auto p = pcomb::builder(pcomb::str("hello")).withErrCheckpt().build();

//   pcomb::ParseResult<string> result = p->tryParse("hey", {true});

//   assertEmptyParseResult(result, "hey", "\"hello\"");
//   verifyExecLog(result, false, 3, 0);
// }


int main() { return runTests(); }
