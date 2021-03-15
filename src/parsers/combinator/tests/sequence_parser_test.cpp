#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;

auto P_INT = pcomb::integerShared();
auto P_HELLO = pcomb::strShared("hello");

TEST(success_exact) {
  auto p = pcomb::seq(P_INT, P_HELLO);
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hello");
  assertParseResult(result, expected, "");
}

TEST(success_leftover_verbose) {
  auto p = pcomb::seq(P_INT, P_HELLO);
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hellogoodbye", {true});
  assertParseResult(result, expected, "goodbye");
  verifyExecLog(result, true, 15, 2);
}

TEST(success_empty) {
  auto p = pcomb::seq();
  auto expected = std::tuple<>();

  auto result = p->tryParse("123hello");
  assertParseResult(result, expected, "123hello");
}

TEST(success_one) {
  auto p = pcomb::seq(P_INT);
  auto expected = std::make_tuple(12345);

  auto result = p->tryParse("12345a");
  assertParseResult(result, expected, "a");
}

TEST(success_many) {
  auto p = pcomb::seq(P_INT, P_HELLO, P_INT, P_HELLO, P_HELLO);
  auto expected = std::make_tuple(123, "hello"s, 123, "hello"s, "hello"s);

  auto result = p->tryParse("123hello123hellohello");
  assertParseResult(result, expected, "");
}

TEST(success_assumesOwnershipOfParsers) {
  // Making sure that seq parser is assuming ownership of the parser ptrs it is passed.
  auto makeSeq = []() { return pcomb::seq(P_INT, pcomb::str("hello")); };
  auto p = makeSeq();
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hello");
  assertParseResult(result, expected, "");
}

TEST(failure_firstMismatched_withErrCheckpt_verbose) {
  auto p = pcomb::create(pcomb::seq(P_INT, P_HELLO)).withErrCheckpt().build();

  auto result = p->tryParse("hey", {true});
  assertEmptyParseResult(result, "hey", "Seq");
  // Short-circuits, so P_HELLO is not executed.
  verifyExecLog(result, false, 3, 1);
}

TEST(failure_otherMismatched_withErrCheckpt_verbose) {
  auto p = pcomb::create(pcomb::seq(P_INT, P_HELLO)).withErrCheckpt().build();

  auto result = p->tryParse("123goodbye", {true});
  assertEmptyParseResult(result, "123goodbye", "Seq");
  verifyExecLog(result, false, 10, 2);
}

TEST(failure_withErrCheckpt_subparserHasErrCheckpt_truncatesRest) {
  auto pHelloMarksErrors = pcomb::create(pcomb::str("hello")).withErrCheckpt().build();
  auto p = pcomb::create(pcomb::seq(P_INT, std::move(pHelloMarksErrors))).withErrCheckpt().build();

  auto result = p->tryParse("123goodbye");
  assertEmptyParseResult(result, "goodbye", "\"hello\"");
}


int main() { return runTests(); }
