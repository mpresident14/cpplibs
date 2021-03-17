#ifndef PREZ_PARSERS_COMBINATOR_PARSERS_HPP
#define PREZ_PARSERS_COMBINATOR_PARSERS_HPP

#include "src/parsers/combinator/alt_parser.hpp"
#include "src/parsers/combinator/more_type_traits.hpp"
#include "src/parsers/combinator/num_parser.hpp"
#include "src/parsers/combinator/parser.hpp"
#include "src/parsers/combinator/parser_builder.hpp"
#include "src/parsers/combinator/sequence_parser.hpp"
#include "src/parsers/combinator/string_parser.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace prez {
namespace pcomb {

using namespace detail;

template <ParserPtr P>
ParserBuilder<P> builder(P&& parser) {
  return ParserBuilder(std::forward<P>(parser));
}

std::unique_ptr<Parser<std::string>> str(std::string_view sv) {
  return std::make_unique<StringParser>(sv);
}

std::shared_ptr<Parser<std::string>> strShared(std::string_view sv) {
  return std::make_shared<StringParser>(sv);
}

// TODO: These can be specialized to a const shared_ptr for common types (e.g. int, long, uint,
// ulong, double, float, etc)
template <typename Integer = int>
requires std::is_integral_v<Integer> std::unique_ptr<Parser<Integer>> integer(int base = 10) {
  return std::make_unique<IntegerParser<Integer>>(base);
}

template <typename Integer = int>
requires std::is_integral_v<Integer> std::shared_ptr<Parser<Integer>> integerShared(int base = 10) {
  return std::make_shared<IntegerParser<Integer>>(base);
}

// TODO: Enable when clang gets floating-pt support for from_chars
// template <typename Decimal = float>
// std::unique_ptr<Parser<Decimal>> decimal(std::chars_format fmt = std::chars_format::general) {
//   return std::make_unique<DecimalParser<Decimal>>(fmt);
// }

template <ParserPtr... Ps>
std::unique_ptr<Parser<std::tuple<pcomb_result_t<Ps>...>>> seq(Ps&&... parsers) {
  // SequenceParser needs to assume ownership, so remove the references
  return std::make_unique<SequenceParser<std::remove_reference_t<Ps>...>>(
      std::forward<Ps>(parsers)...);
}

template <typename T, ParserPtr... Ps>
std::unique_ptr<Parser<T>> alt(Ps&&... parsers) {
  return std::make_unique<AltParser<T, std::remove_reference_t<Ps>...>>(
      std::forward<Ps>(parsers)...);
}

} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSERS_HPP
