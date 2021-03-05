#ifndef PREZ_PARSERS_COMBINATOR_PARSERS_HPP
#define PREZ_PARSERS_COMBINATOR_PARSERS_HPP

#include "src/parsers/combinator/num_parser.hpp"
#include "src/parsers/combinator/parser.hpp"
#include "src/parsers/combinator/string_parser.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace prez {
namespace pcomb {
using namespace detail;

std::unique_ptr<Parser<std::string>> str(std::string_view sv) {
  return std::make_unique<StringParser>(sv);
}

// TODO: These can be specialized to a const shared_ptr for common types (e.g. int, long, uint,
// ulong, double, float, etc)
template <typename Integer = int>
std::unique_ptr<Parser<Integer>> integer(int base = 10) {
  return std::make_unique<IntegerParser<Integer>>(base);
}

// TODO: Enable when clang gets floating-pt support for from_chars
// template <typename Decimal = float>
// std::unique_ptr<Parser<Decimal>> decimal(std::chars_format fmt = std::chars_format::general) {
//   return std::make_unique<DecimalParser<Decimal>>(fmt);
// }


} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSERS_HPP
