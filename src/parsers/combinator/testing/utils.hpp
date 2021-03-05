#ifndef PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
#define PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP

#include "src/parsers/combinator/parsers.hpp"
#include "src/testing/unit_test.hpp"

#include <string_view>

template <typename T>
void assertEmptyParseResult(const prez::pcomb::ParseResult<T>& result, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertFalse(result.obj.has_value());
}

template <typename T>
void assertParseResult(
    const prez::pcomb::ParseResult<T>& result, const T& obj, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertEquals(obj, result.obj.value());
}

#endif // PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
