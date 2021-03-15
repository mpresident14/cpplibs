// #ifndef PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP
// #define PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP

// #include "src/parsers/combinator/parser.hpp"
// #include "src/parsers/combinator/typing.hpp"

// #include <string>
// #include <tuple>

// namespace prez {
// namespace pcomb {
// namespace detail {

// template <typename T, typename... Ps>
// concept ResultsConvertibleTo = (true && ... && std::is_convertible_v<pcomb_result_t<Ps>, T>);


// template <typename T, ParserPtr... Ps> requires ResultsConvertibleTo<T, Ps...>
// class AltParser : public Parser<T> {

// public:
//   AltParser(Ps&&... parsers) : Parser<R>(DEFAULT_NAME), parsers_(std::forward<Ps>(parsers)...) {}

//   ParseResult<R> tryParse(std::string_view input) override {
//     return tryParseImpl(input, std::index_sequence_for<Ps...>{});
//   }

// private:
//   // TODO: Default name should be
//   static constexpr char DEFAULT_NAME[4] = "Alt";

//   struct FailureInfo {
//     ParseResult<T> result;
//     std::vector<std::string> failureChain;
//   };

//   template <size_t... Is>
//   ParseResult<T> tryParseImpl(std::string_view input, std::index_sequence<Is...> indexSeq) {
//     FailureInfo failureInfo{false, input, {}};
//     trySingleParse<Is>(input, failureInfo)...;
//     if (failureInfo.result.success) {
//       return std::move(result);
//     }

//     failureInfo.failureChain.push_back(this->name_);
//       return ParseResult<R>{false, std::move(failureInfo.failureChain), failureInfo.rest};
//   }

//   template <size_t I>
//   auto trySingleParse(std::string_view& input, FailureInfo& failureInfo) {
//     auto& parser = std::get<I>(parsers_);

//     if (failureInfo.alreadySucceeded) {
//       using ParseResultType = decltype(parser->tryParse(input));
//       return ParseResultType{false, std::vector<std::string>(), input};
//     }

//     auto parseResult = parser->tryParse(input);
//     if (parseResult.success) {
//       input = parseResult.rest;
//       return parseResult;
//     }

//     failureInfo.failureChain =
//         std::move(std::get<std::vector<std::string>>(parseResult.objOrErrorChain));
//     failureInfo.alreadySucceeded = true;
//     if (parser->hasErrCheckpt()) {
//       failureInfo.rest = parseResult.rest;
//     }

//     return parseResult;
//   }

//   template <typename... Results, size_t... Is>
//   bool allSucceeded(const std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
//     return (true && ... && std::get<Is>(parseResults).success);
//   }

//   template <typename... Results, size_t... Is>
//   R combineResults(std::tuple<Results...>& parseResults, const std::index_sequence<Is...>&) {
//     return std::make_tuple(std::move(std::get<0>(std::get<Is>(parseResults).objOrErrorChain))...);
//   }


//   std::tuple<Ps...> parsers_;
// };

// } // namespace detail
// } // namespace pcomb
// } // namespace prez

// #endif // PREZ_PARSERS_COMBINATOR_ALT_PARSER_HPP
