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

  ParseResult<R> tryParse(std::string_view input) override {
    return tryParseImpl(input, std::index_sequence_for<Ps...>{});
  }

protected:
  const std::string& getDefaultName() const override {
    return getDefaultNameImpl(std::index_sequence_for<Ps...>{});
  }

private:
  static const std::string EMPTY_NAME;
  static constexpr char AMPERSANDS[] = " && ";

  struct FailureInfo {
    bool alreadyFailed;
    std::string_view rest;
    std::vector<std::string> failureChain;
  };

  template <size_t... Is>
  ParseResult<R> tryParseImpl(std::string_view input, std::index_sequence<Is...> indexSeq) {
    FailureInfo failureInfo{false, input, {}};
    auto parseResults = std::make_tuple(trySingleParse<Is>(input, failureInfo)...);
    if (failureInfo.alreadyFailed) {
      failureInfo.failureChain.push_back(this->name_);
      return ParseResult<R>{false, std::move(failureInfo.failureChain), failureInfo.rest};
    }

    return ParseResult<R>{true, combineResults(parseResults, indexSeq), input};
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
  R combineResults(std::tuple<Results...>& parseResults, std::index_sequence<Is...>) {
    return std::make_tuple(std::move(std::get<0>(std::get<Is>(parseResults).objOrErrorChain))...);
  }

  template <size_t... Is>
  const std::string& getDefaultNameImpl(std::index_sequence<Is...>) const {
    if constexpr (sizeof...(Is) == 0) {
      return EMPTY_NAME;
    }

    std::string str("Seq [ ");
    (..., str.append(std::get<Is>(parsers_)->getName()).append(AMPERSANDS));
    str.erase(str.size() - sizeof(AMPERSANDS) - 1);
    return str.append(" ]");
  }


  std::tuple<Ps...> parsers_;
};

template <ParserPtr... Ps>
const std::string SequenceParser<Ps...>::EMPTY_NAME = "Empty Seq";

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_SEQUENCE_PARSER_HPP
