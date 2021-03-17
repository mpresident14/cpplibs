#ifndef PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP

#include "src/misc/more_type_traits.hpp"
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
        parseResult.rest,
        std::move(parseResult.failedParserName),
        this->makeExeLog(options, input, false, std::move(parseResult.executionLog))};
  }

protected:
  std::string getDefaultName() const override { return "Map"; }

private:
  P parser_;
  F mapFn_;
};

// template <ParserPtr P, typename F>
// requires misc::is_specialization_v<pcomb_result_t<P>, std::tuple> class MapParser
//     : public Parser<invoke_on_tuple_args_t<F, pcomb_result_t<P>>> {
//   using typename Parser<invoke_on_tuple_args_t<F, pcomb_result_t<P>>>::result_type;

// public:
//   MapParser(std::string_view sv) : str_(sv) {}

//   ParseResult<result_type> tryParse(std::string_view input, const ParseOptions& options) override
//   {
//     if (input.starts_with(str_)) {
//       return {str_, input.substr(str_.size()), {}, this->makeExeLog(options, input, true)};
//     }
//     return {
//         {},
//         this->restIfCheckpted(input),
//         this->getNameForFailure(),
//         this->makeExeLog(options, input, false)};
//   }

// protected:
//   std::string getDefaultName() const override {
//     return std::string(1, '"').append(str_).append(1, '"');
//   }

// private:
//   std::string str_;
// };

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_MAP_PARSER_HPP
