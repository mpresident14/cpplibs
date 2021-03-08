#ifndef PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
#define PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP

#include "src/parsers/combinator/parsers.hpp"
#include "src/testing/unit_test.hpp"

#include <string_view>

template <typename T>
void assertEmptyParseResult(const prez::pcomb::ParseResult<T>& result, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertFalse(result.success);
}

template <typename T>
void assertParseResult(
    const prez::pcomb::ParseResult<T>& result, const T& obj, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertTrue(result.success);
  prez::unit_test::assertEquals(obj, std::get<T>(result.objOrErrorChain));
}

#endif // PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
