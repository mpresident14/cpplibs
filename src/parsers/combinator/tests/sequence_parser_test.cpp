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

TEST(success_leftover) {
  auto p = pcomb::seq(P_INT, P_HELLO);
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hellogoodbye");
  assertParseResult(result, expected, "goodbye");
}

TEST(failure_firstMismatched) {
  auto p = pcomb::seq(P_INT, P_HELLO);

  auto result = p->tryParse("hey");
  assertEmptyParseResult(result, "hey");
}

TEST(failure_otherMismatched) {
  auto p = pcomb::seq(P_INT, P_HELLO);

  auto result = p->tryParse("123goodbye");
  assertEmptyParseResult(result, "123goodbye");
}

TEST(failure_marksErrors_truncatesRest) {
  auto pHelloMarksErrors = pcomb::str("hello");
  pHelloMarksErrors->markErrors();
  auto p = pcomb::seq(P_INT, std::move(pHelloMarksErrors));

  auto result = p->tryParse("123goodbye");
  assertEmptyParseResult(result, "goodbye");
}

TEST(assumesOwnershipOfParsers) {
  // Making sure that seq parser is assuming ownership of the parser ptrs it is passed.
  auto makeSeq = []() {
    return pcomb::seq(P_INT, pcomb::str("hello"));
  };
  auto p = makeSeq();
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hello");
  assertParseResult(result, expected, "");
}


int main() { return runTests(); }
