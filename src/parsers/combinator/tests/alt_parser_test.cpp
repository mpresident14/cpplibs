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

// TEST(success_leftover) {
//   auto p = pcomb::seq(P_INT, P_HELLO);
//   auto expected = std::make_tuple(123, "hello"s);

//   auto result = p->tryParse("123hellogoodbye");
//   assertParseResult(result, expected, "goodbye");
// }

// TEST(success_empty) {
//   auto p = pcomb::seq();
//   auto expected = std::tuple<>();

//   auto result = p->tryParse("123hello");
//   assertParseResult(result, expected, "123hello");
// }

// TEST(success_one) {
//   auto p = pcomb::seq(P_INT);
//   auto expected = std::make_tuple(12345);

//   auto result = p->tryParse("12345a");
//   assertParseResult(result, expected, "a");
// }

// TEST(success_many) {
//   auto p = pcomb::seq(P_INT, P_HELLO, P_INT, P_HELLO, P_HELLO);
//   auto expected = std::make_tuple(123, "hello"s, 123, "hello"s, "hello"s);

//   auto result = p->tryParse("123hello123hellohello");
//   assertParseResult(result, expected, "");
// }

// TEST(failure_firstMismatched) {
//   auto p = pcomb::seq(P_INT, P_HELLO);

//   auto result = p->tryParse("hey");
//   assertEmptyParseResult(result, "hey");
// }

// TEST(failure_otherMismatched) {
//   auto p = pcomb::seq(P_INT, P_HELLO);

//   auto result = p->tryParse("123goodbye");
//   assertEmptyParseResult(result, "123goodbye");
// }

// TEST(failure_hasErrCheckpt_truncatesRest) {
//   auto pHelloMarksErrors = pcomb::str("hello");
//   pHelloMarksErrors->setErrCheckpt();
//   auto p = pcomb::seq(P_INT, std::move(pHelloMarksErrors));

//   auto result = p->tryParse("123goodbye");
//   assertEmptyParseResult(result, "goodbye");
// }

// TEST(assumesOwnershipOfParsers) {
//   // Making sure that seq parser is assuming ownership of the parser ptrs it is passed.
//   auto makeSeq = []() {
//     return pcomb::seq(P_INT, pcomb::str("hello"));
//   };
//   auto p = makeSeq();
//   auto expected = std::make_tuple(123, "hello"s);

//   auto result = p->tryParse("123hello");
//   assertParseResult(result, expected, "");
// }


int main() { return runTests(); }
