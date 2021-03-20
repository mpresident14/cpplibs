#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;

auto P_HELLO = pcomb::strShared("hello");
auto P_INT = pcomb::integerShared();
auto P_SEQ = pcomb::seqShared(P_INT, P_HELLO);
auto ADD_LEN = [](int n, const string& str) { return n + str.length(); };

TEST(success_exact) {
  auto p = pcomb::mapTuple(P_SEQ, ADD_LEN);

  auto result = p->tryParse("123hello");

  assertParseResult(result, 128ul, "");
  assertEquals(nullptr, result.executionLog);
}

TEST(success_leftover_verbose) {
  const char input[] = "123helloworld";
  const char expectedRest[] = "world";
  auto p = pcomb::mapTuple(P_SEQ, ADD_LEN);

  auto result = p->tryParse("123helloworld", {true});

  assertParseResult(result, 128ul, expectedRest);
  verifyExecLog(result, true, "MapTuple", input, expectedRest, 1);
}

TEST(failure_mismatched) {
  auto p = pcomb::mapTuple(P_SEQ, ADD_LEN);

  auto result = p->tryParse("hey");

  assertEmptyParseResult(result, "");
  assertEquals(nullptr, result.executionLog);
}

TEST(failure_mismatched_withErrCheckpt_verbose) {
  const char input[] = "hey";
  const char name[] = "CustomName";
  auto p = pcomb::builder(pcomb::mapTuple(P_SEQ, ADD_LEN)).withName(name).withErrCheckpt().build();

  auto result = p->tryParse(input, {true});

  assertEmptyParseResult(result, input, name);
  verifyExecLog(result, false, name, input, input, 1);
}

TEST(failure_mismatched_subparserWithErrCheckpt) {
  auto p = pcomb::mapTuple(
      pcomb::seq(P_HELLO, pcomb::builder(P_INT).withErrCheckpt().build()),
      [](const string& str, int n) { return n + str.length(); });

  auto result = p->tryParse("hellogoodbye");

  assertEmptyParseResult(result, "goodbye", "Integer");
  assertEquals(nullptr, result.executionLog);
}


int main() { return runTests(); }
