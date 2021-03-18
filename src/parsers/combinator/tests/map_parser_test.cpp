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

TEST(success_leftover_verbose) {
  auto p = pcomb::map(P_HELLO, GET_SIZE);

  auto result = p->tryParse("helloworld", {true});

  assertParseResult(result, 5ul, "world");
  verifyExecLog(result, true, 10, 1);
}

TEST(failure_mismatched) {
  auto p = pcomb::map(P_HELLO, GET_SIZE);

  auto result = p->tryParse("hey");

  assertEmptyParseResult(result, "");
  assertEquals(nullptr, result.executionLog);
}

TEST(failure_mismatched_withErrCheckpt_verbose) {
  auto p = pcomb::builder(pcomb::map(P_HELLO, GET_SIZE)).withErrCheckpt().build();

  auto result = p->tryParse("hey", {true});

  assertEmptyParseResult(result, "hey", "Map");
  verifyExecLog(result, false, 3, 1);
}

TEST(failure_mismatched_subparserWithErrCheckpt) {
  auto p = pcomb::map(
      pcomb::seq(P_HELLO, pcomb::builder(P_HELLO).withErrCheckpt().build()),
      [](const auto&) { return 3; });

  auto result = p->tryParse("hellogoodbye");

  assertEmptyParseResult(result, "goodbye", "\"hello\"");
  assertEquals(nullptr, result.executionLog);
}


int main() { return runTests(); }
