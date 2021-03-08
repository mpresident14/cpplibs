#include "src/parsers/combinator/parsers.hpp"
#include "src/parsers/combinator/testing/utils.hpp"
#include "src/testing/unit_test.hpp"

using namespace std;
using namespace prez::unit_test;
using namespace prez;


TEST(success_exact) {
  auto pInt = pcomb::integer();
  auto pHello = pcomb::str("hello");
  auto p = pcomb::seq(pInt, pHello);
  auto expected = std::make_tuple(123, "hello"s);

  auto result = p->tryParse("123hello");
  assertParseResult(result, expected, "");
}

// TEST(success_exact_hex) {
//   auto p = pcomb::integer(16);

//   pcomb::ParseResult<int> result = p->tryParse("FF");
//   assertParseResult(result, 0xFF, "");
// }


// TEST(success_leftover) {
//   auto p = pcomb::integer();

//   pcomb::ParseResult<int> result = p->tryParse("123hello");
//   assertParseResult(result, 123, "hello");
// }

// TEST(failure_mismatched) {
//   auto p = pcomb::integer(16);

//   pcomb::ParseResult<int> result = p->tryParse("hey");
//   assertEmptyParseResult(result, "hey");
// }


int main() { return runTests(); }
