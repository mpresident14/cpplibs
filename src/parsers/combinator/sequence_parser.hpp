#ifndef PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP


#include "src/parsers/combinator/parser.hpp"

#include <string>
#include <tuple>
#include <unordered_set>

namespace prez {
namespace pcomb {
namespace detail {

template <typename ParserPtr>
struct pcomb_result {
  using type = typename std::decay_t<decltype(*std::declval<ParserPtr>())>::result_type;
};

template <typename ParserPtr>
using pcomb_result_t = typename pcomb_result<ParserPtr>::type;

// TODO: Update for empty tuple of parser ptrs
template <typename... ParserPtrs>
class SequenceParser : public Parser<std::tuple<pcomb_result_t<ParserPtrs>...>> {
  // TODO: Use Parser::result_type here ?
  using R = typename Parser<std::tuple<pcomb_result_t<ParserPtrs>...>>::result_type;

public:
  SequenceParser(ParserPtrs&&... parsers) : parsers_(std::forward<ParserPtrs>(parsers)...) {}

  ParseResult<R> tryParse(std::string_view input) override {
    return tryParseImpl(input, std::index_sequence_for<ParserPtrs...>{});
  }

private:
  static const size_t PARSE_OK = (size_t)-1;

  template <size_t... Is>
  ParseResult<R> tryParseImpl(std::string_view input, std::index_sequence<Is...> indexSeq) {
    size_t failureIndex = PARSE_OK;
    auto parseResults = std::make_tuple(trySingleParse<Is>(failureIndex, input)...);
    if (allSucceeded(parseResults, indexSeq)) {
      return ParseResult<R>{{combineResults(parseResults, indexSeq)}, input};
    }

    return ParseResult<R>{{}, input};
  }

  template <size_t I>
  auto trySingleParse(size_t& failureIndex, std::string_view& input) {
    using ParseResultType = decltype(std::get<I>(parsers_)->tryParse(input));

    if (failureIndex != PARSE_OK) {
      return ParseResultType{{}, input};
    }

    auto parseResult = std::get<I>(parsers_)->tryParse(input);
    if (parseResult.obj) {
      input = parseResult.rest;
      return parseResult;
    }

    failureIndex = I;
    // TODO: Figure out how rest should be populated based on error names
    input = parseResult.rest;
    return parseResult;
  }

  template <typename... Results, size_t... Is>
  bool allSucceeded(const std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
    std::unordered_set<bool> booleans = {std::get<Is>(parseResults).obj.has_value()...};
    return booleans.size() <= 1 && *booleans.cbegin();
  }

  template <typename... Results, size_t... Is>
  R combineResults(std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
    return std::make_tuple(std::move(*std::get<Is>(parseResults).obj)...);
  }


  std::tuple<ParserPtrs...> parsers_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
