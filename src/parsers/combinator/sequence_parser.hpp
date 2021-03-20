#ifndef PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP


#include "src/parsers/combinator/more_type_traits.hpp"
#include "src/parsers/combinator/parser.hpp"

#include <sstream>
#include <string>
#include <tuple>

namespace prez {
namespace pcomb {
namespace detail {

template <ParserPtr... Ps>
class SequenceParser : public Parser<std::tuple<pcomb_result_t<Ps>...>> {
  using typename Parser<std::tuple<pcomb_result_t<Ps>...>>::result_type;

public:
  SequenceParser(Ps... parsers) : parsers_(std::move(parsers)...) {}

  ParseResult<result_type> tryParse(std::string_view input, const ParseOptions& options) override {
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
  ParseResult<result_type> tryParseImpl(
      std::string_view input, const ParseOptions& options, std::index_sequence<Is...> indexSeq) {
    std::string_view originalInput = input;
    FailureInfo failureInfo{false, input, {}};
    // Braced initializers to guarantee order of parameter evaluation:
    // https://stackoverflow.com/a/42047998
    auto parseResults = std::tuple<decltype(std::get<Is>(parsers_)->tryParse(input))...>{
        trySingleParse<Is>(input, options, failureInfo)...};

    failureInfo.failedParserName.has_value() ? failureInfo.rest
                                             : this->restIfCheckpted(originalInput);
    if (failureInfo.alreadyFailed) {
      return {
          {},
          // If none of the subparsers were checkpointed, use the failure info for this parser.
          failureInfo.failedParserName.has_value() ? failureInfo.rest
                                                   : this->restIfCheckpted(originalInput),
          failureInfo.failedParserName.has_value() ? std::move(failureInfo.failedParserName)
                                                   : this->getNameIfCheckpted(),
          makeExeLogFromVec(
              options,
              originalInput,
              originalInput,
              false,
              getExecutionLogs(parseResults, indexSeq))};
    }

    // We passed 'input' by reference and updated it, so it is now represents 'rest'
    return {
        resultObjsToTuple(parseResults, indexSeq),
        input,
        {},
        makeExeLogFromVec(
            options, originalInput, input, true, getExecutionLogs(parseResults, indexSeq))};
  }

  template <size_t I>
  auto
  trySingleParse(std::string_view& input, const ParseOptions& options, FailureInfo& failureInfo) {
    auto& parser = std::get<I>(parsers_);
    using ParseResultType = decltype(parser->tryParse(input));

    // Short circuit after initial failure
    if (failureInfo.alreadyFailed) {
      return ParseResultType{{}, input, {}, nullptr};
    }

    ParseResultType parseResult = parser->tryParse(input, options);
    if (parseResult.obj.has_value()) {
      input = parseResult.rest;
      return parseResult;
    }

    failureInfo.alreadyFailed = true;
    // Use last checkpointed failure for error info. We provide for specific details with the
    // 'verbose' option.
    if (parseResult.failedParserName.has_value()) {
      failureInfo.rest = parseResult.rest;
      failureInfo.failedParserName = std::move(parseResult.failedParserName);
    }

    return parseResult;
  }

  template <typename... Results, size_t... Is>
  result_type resultObjsToTuple(std::tuple<Results...>& parseResults, std::index_sequence<Is...>) {
    return result_type{std::move(*std::get<Is>(parseResults).obj)...};
  }

  template <typename... Results, size_t... Is>
  std::vector<std::unique_ptr<ExecutionLog>>
  getExecutionLogs(std::tuple<Results...>& parseResults, std::index_sequence<Is...>) {
    std::vector<std::unique_ptr<ExecutionLog>> executionLogs;
    executionLogs.reserve(sizeof...(Is));
    (..., pushBackIfNonNull(executionLogs, std::move(std::get<Is>(parseResults).executionLog)));
    return executionLogs;
  }

  static void pushBackIfNonNull(
      std::vector<std::unique_ptr<ExecutionLog>>& executionLogs,
      std::unique_ptr<ExecutionLog>&& executionLog) {
    if (executionLog) {
      executionLogs.push_back(std::move(executionLog));
    }
  }

  std::unique_ptr<ExecutionLog> makeExeLogFromVec(
      const ParseOptions& options,
      std::string_view input,
      std::string_view rest,
      bool success,
      std::vector<std::unique_ptr<ExecutionLog>> children) {
    if (options.verbose) {
      return std::make_unique<ExecutionLog>(
          std::move(children), this->getName(), input, rest, success);
    }
    return nullptr;
  }


  std::tuple<Ps...> parsers_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
