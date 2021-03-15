#ifndef PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP


#include "src/parsers/combinator/parser.hpp"
#include "src/parsers/combinator/typing.hpp"

#include <sstream>
#include <string>
#include <tuple>

namespace prez {
namespace pcomb {
namespace detail {

template <ParserPtr... Ps>
class SequenceParser : public Parser<std::tuple<pcomb_result_t<Ps>...>> {
  using R = std::tuple<pcomb_result_t<Ps>...>;

public:
  SequenceParser(Ps&&... parsers) : parsers_(std::forward<Ps>(parsers)...) {}

  ParseResult<R> tryParse(std::string_view input, const ParseOptions& options) override {
    return tryParseImpl(input, options, std::index_sequence_for<Ps...>{});
  }

protected:
  std::string getDefaultName() const override { return "Seq"; }

private:
  struct FailureInfo {
    bool alreadyFailed;
    std::string_view rest;
    std::optional<std::string> failedParserName;
  };

  template <size_t... Is>
  ParseResult<R> tryParseImpl(
      std::string_view input, const ParseOptions& options, std::index_sequence<Is...> indexSeq) {
    std::string_view originalInput = input;
    FailureInfo failureInfo{false, input, {}};
    // Braced initializers to guarantee ordering: https://stackoverflow.com/a/42047998
    auto parseResults = std::tuple<decltype(std::get<Is>(parsers_)->tryParse(input))...>{
        trySingleParse<Is>(input, options, failureInfo)...};

    if (failureInfo.alreadyFailed) {
      return ParseResult<R>{
          {},
          // If none of the subparsers were checkpointed, use the failure info for this parser.
          failureInfo.failedParserName.has_value() ? failureInfo.rest
                                                   : this->restIfCheckpted(originalInput),
          failureInfo.failedParserName.has_value() ? std::move(failureInfo.failedParserName)
                                                   : this->getNameForFailure(),
          this->makeExeLog(
              options, originalInput, false, getExecutionLogs(parseResults, indexSeq))};
    }

    return ParseResult<R>{
        resultObjsToTuple(parseResults, indexSeq),
        input,
        {},
        this->makeExeLog(options, originalInput, true, getExecutionLogs(parseResults, indexSeq))};
  }

  template <size_t I>
  auto
  trySingleParse(std::string_view& input, const ParseOptions& options, FailureInfo& failureInfo) {
    auto& parser = std::get<I>(parsers_);

    // Short circuit after initial failure
    if (failureInfo.alreadyFailed) {
      using ParseResultType = decltype(parser->tryParse(input));
      return ParseResultType{{}, input, {}, nullptr};
    }

    auto parseResult = parser->tryParse(input, options);
    if (parseResult.obj.has_value()) {
      input = parseResult.rest;
      return parseResult;
    }

    failureInfo.alreadyFailed = true;
    // Use last checkpointed failure for error info. We provide for specific details with the
    // 'verbose' option.
    if (parser->hasErrCheckpt()) {
      failureInfo.rest = parseResult.rest;
      failureInfo.failedParserName = std::move(parseResult.failedParserName);
    }

    return parseResult;
  }

  template <typename... Results, size_t... Is>
  R resultObjsToTuple(std::tuple<Results...>& parseResults, std::index_sequence<Is...>) {
    return R{std::move(*std::get<Is>(parseResults).obj)...};
  }

  template <typename... Results, size_t... Is>
  std::vector<std::unique_ptr<ExecutionLog>>
  getExecutionLogs(std::tuple<Results...>& parseResults, std::index_sequence<Is...>) {
    std::vector<std::unique_ptr<ExecutionLog>> executionLogs;
    executionLogs.reserve(sizeof...(Is));
    (..., pushBackIfNonNull(executionLogs, std::move(std::get<Is>(parseResults).executionLog)));
    return executionLogs;
  }

  void pushBackIfNonNull(
      std::vector<std::unique_ptr<ExecutionLog>>& executionLogs,
      std::unique_ptr<ExecutionLog>&& executionLog) {
    if (executionLog) {
      executionLogs.push_back(std::move(executionLog));
    }
  }


  std::tuple<Ps...> parsers_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
