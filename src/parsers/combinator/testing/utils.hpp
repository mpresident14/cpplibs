#ifndef PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
#define PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP

#include "src/parsers/combinator/parsers.hpp"
#include "src/testing/unit_test.hpp"

#include <string_view>

template <typename T>
void assertParseResult(
    const prez::pcomb::ParseResult<T>& result, const T& obj, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertFalse(result.failedParserName.has_value());
  prez::unit_test::assertEquals(obj, result.obj.value());
}

template <typename T>
void assertEmptyParseResult(const prez::pcomb::ParseResult<T>& result, std::string_view rest) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertFalse(result.obj.has_value());
  prez::unit_test::assertFalse(result.failedParserName.has_value());
}

template <typename T>
void assertEmptyParseResult(
    const prez::pcomb::ParseResult<T>& result,
    std::string_view rest,
    std::string_view failedParserName) {
  prez::unit_test::assertEquals(rest, result.rest);
  prez::unit_test::assertFalse(result.obj.has_value());
  prez::unit_test::assertEquals(failedParserName, result.failedParserName.value());
}

template <typename T>
void verifyExecLog(
    const prez::pcomb::ParseResult<T>& result, bool success, size_t inputSize, size_t numChildren) {
  prez::unit_test::assertEquals(success, result.executionLog->success);
  prez::unit_test::assertEquals(inputSize, result.executionLog->inputSize);
  prez::unit_test::assertEquals(numChildren, result.executionLog->children.size());
}


#endif // PREZ_PARSERS_COMBINATOR_TESTING_UTILS_HPP
