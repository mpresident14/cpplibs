#ifndef PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP


#include "src/parsers/combinator/parser.hpp"
#include "src/parsers/combinator/typing.hpp"

#include <iostream>
#include <string>
#include <tuple>

namespace prez {
namespace pcomb {
namespace detail {

template <ParserPtr P>
struct pcomb_result {
  using type = typename std::decay_t<decltype(*std::declval<P>())>::result_type;
};

template <ParserPtr P>
using pcomb_result_t = typename pcomb_result<P>::type;

template <ParserPtr... Ps>
class SequenceParser : public Parser<std::tuple<pcomb_result_t<Ps>...>> {
  using R = std::tuple<pcomb_result_t<Ps>...>;

public:
  SequenceParser(Ps&&... parsers)
      : Parser<R>(DEFAULT_NAME), parsers_(std::forward<Ps>(parsers)...) {}

  ParseResult<R> tryParse(std::string_view input) override {
    return tryParseImpl(input, std::index_sequence_for<Ps...>{});
  }

private:
  static constexpr char DEFAULT_NAME[9] = "Sequence";

  struct FailureInfo {
    bool alreadyFailed;
    std::string_view rest;
    std::vector<std::string> failureChain;
  };

  template <size_t... Is>
  ParseResult<R> tryParseImpl(std::string_view input, std::index_sequence<Is...> indexSeq) {
    FailureInfo failureInfo{false, input, {}};
    auto parseResults = std::make_tuple(trySingleParse<Is>(input, failureInfo)...);
    if (allSucceeded(parseResults, indexSeq)) {
      return ParseResult<R>{true, combineResults(parseResults, indexSeq), input};
    }

    failureInfo.failureChain.push_back(this->name_);
    return ParseResult<R>{false, std::move(failureInfo.failureChain), failureInfo.rest};
  }

  template <size_t I>
  auto trySingleParse(std::string_view& input, FailureInfo& failureInfo) {
    auto& parser = std::get<I>(parsers_);

    if (failureInfo.alreadyFailed) {
      using ParseResultType = decltype(parser->tryParse(input));
      return ParseResultType{false, std::vector<std::string>(), input};
    }

    auto parseResult = parser->tryParse(input);
    if (parseResult.success) {
      input = parseResult.rest;
      return parseResult;
    }

    failureInfo.failureChain =
        std::move(std::get<std::vector<std::string>>(parseResult.objOrErrorChain));
    failureInfo.alreadyFailed = true;
    if (parser->hasErrCheckpt()) {
      failureInfo.rest = parseResult.rest;
    }

    return parseResult;
  }

  template <typename... Results, size_t... Is>
  bool allSucceeded(const std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
    return (true && ... && std::get<Is>(parseResults).success);
  }

  template <typename... Results, size_t... Is>
  R combineResults(std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
    return std::make_tuple(std::move(std::get<0>(std::get<Is>(parseResults).objOrErrorChain))...);
  }


  std::tuple<Ps...> parsers_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
