#ifndef PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP

#include "src/parsers/combinator/more_type_traits.hpp"
#include "src/parsers/combinator/parser.hpp"

#include <string>
#include <tuple>

namespace prez {
namespace pcomb {
namespace detail {

template <typename T, typename... Ps>
concept ResultsConvertibleTo = (true && ... && std::is_convertible_v<pcomb_result_t<Ps>, T>);


template <typename T, ParserPtr... Ps>
requires ResultsConvertibleTo<T, Ps...> class AltParser : public Parser<T> {

public:
  AltParser(Ps... parsers) : parsers_(std::move(parsers)...) {}

  ParseResult<T> tryParse(std::string_view input, const ParseOptions& options) override {
    return tryParseImpl(input, options, std::index_sequence_for<Ps...>{});
  }

protected:
  std::string getDefaultName() const override { return "Alt"; }

private:
  template <size_t... Is>
  ParseResult<T>
  tryParseImpl(std::string_view input, const ParseOptions& options, std::index_sequence<Is...>) {
    ParseResult<T> overallParseResult{{}, "", {}, this->makeExeLog(options, input, input, false)};
    (..., trySingleParse<Is>(input, options, overallParseResult));
    if (overallParseResult.obj.has_value() || overallParseResult.failedParserName.has_value()) {
      return overallParseResult;
    }

    // Might need to populate 'rest' and 'failedParserName'
    if (this->hasErrCheckpt_) {
      overallParseResult.rest = input;
      overallParseResult.failedParserName.emplace(this->getName());
    }

    return overallParseResult;
  }

  template <size_t I>
  void trySingleParse(
      std::string_view input, const ParseOptions& options, ParseResult<T>& overallParseResult) {
    auto& parser = std::get<I>(parsers_);
    using ParseResultType = decltype(parser->tryParse(input));

    // Short circuit after initial success
    if (overallParseResult.obj.has_value()) {
      return;
    }

    ParseResultType parseResult = parser->tryParse(input, options);
    if (options.verbose) {
      overallParseResult.executionLog->children.push_back(std::move(parseResult.executionLog));
    }

    if (parseResult.obj.has_value()) {
      overallParseResult.obj.emplace(*parseResult.obj);
      overallParseResult.rest = parseResult.rest;
      if (options.verbose) {
        overallParseResult.executionLog->success = true;
        overallParseResult.executionLog->rest = parseResult.rest;
      }
      return;
    }

    if (parseResult.failedParserName.has_value()) {
      overallParseResult.failedParserName.swap(parseResult.failedParserName);
      overallParseResult.rest = parseResult.rest;
    }
  }


  std::tuple<Ps...> parsers_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP
