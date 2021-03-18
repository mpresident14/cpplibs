#ifndef PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP

#include "src/parsers/combinator/more_type_traits.hpp"
#include "src/parsers/combinator/parser.hpp"

#include <string>
#include <tuple>
#include <type_traits>


namespace prez {
namespace pcomb {
namespace detail {

template <typename F, typename Tuple>
struct invoke_on_tuple_args;

template <typename F, typename... Args>
struct invoke_on_tuple_args<F, std::tuple<Args...>> {
  using type = std::invoke_result_t<F, Args...>;
};

template <typename F, typename Tuple>
using invoke_on_tuple_args_t = typename invoke_on_tuple_args<F, Tuple>::type;


template <ParserPtr P, typename F>
class MapParser : public Parser<std::invoke_result_t<F, pcomb_result_t<P>>> {
  using typename Parser<std::invoke_result_t<F, pcomb_result_t<P>>>::result_type;

public:
  // Templated so we can use universal reference
  template <typename F2 = F>
  MapParser(P parser, F2&& mapFn) : parser_(std::move(parser)), mapFn_(std::forward<F2>(mapFn)) {}

  ParseResult<result_type> tryParse(std::string_view input, const ParseOptions& options) override {
    auto parseResult = parser_->tryParse(input, options);
    if (parseResult.obj.has_value()) {
      return {
          mapFn_(std::move(*parseResult.obj)),
          parseResult.rest,
          {},
          this->makeExeLog(options, input, true, std::move(parseResult.executionLog))};
    }

    return {
        {},
        parseResult.failedParserName.has_value() ? parseResult.rest : this->restIfCheckpted(input),
        parseResult.failedParserName.has_value() ? std::move(parseResult.failedParserName)
                                                 : this->getNameForFailure(),
        this->makeExeLog(options, input, false, std::move(parseResult.executionLog))};
  }

protected:
  std::string getDefaultName() const override { return "Map"; }

private:
  P parser_;
  F mapFn_;
};

template <ParserPtr P, typename F>
class MapTupleParser : public Parser<invoke_on_tuple_args_t<F, pcomb_result_t<P>>> {
  using typename Parser<invoke_on_tuple_args_t<F, pcomb_result_t<P>>>::result_type;

public:
  // Templated so we can use universal reference
  template <typename F2 = F>
  MapTupleParser(P parser, F2&& mapFn)
      : parser_(std::move(parser)), mapFn_(std::forward<F2>(mapFn)) {}

  ParseResult<result_type> tryParse(std::string_view input, const ParseOptions& options) override {
    auto parseResult = parser_->tryParse(input, options);
    if (parseResult.obj.has_value()) {
      return {
          callMapFn(
              *parseResult.obj,
              std::make_index_sequence<
                  std::tuple_size_v<std::remove_reference_t<decltype(*parseResult.obj)>>>{}),
          parseResult.rest,
          {},
          this->makeExeLog(options, input, true, std::move(parseResult.executionLog))};
    }

    return {
        {},
        parseResult.failedParserName.has_value() ? parseResult.rest : this->restIfCheckpted(input),
        parseResult.failedParserName.has_value() ? std::move(parseResult.failedParserName)
                                                 : this->getNameForFailure(),
        this->makeExeLog(options, input, false, std::move(parseResult.executionLog))};
  }

  template <typename... Args, size_t... Is>
  result_type callMapFn(std::tuple<Args...>& args, std::index_sequence<Is...>) {
    return mapFn_(std::get<Is>(std::move(args))...);
  }

protected:
  std::string getDefaultName() const override { return "MapTuple"; }

private:
  P parser_;
  F mapFn_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP
