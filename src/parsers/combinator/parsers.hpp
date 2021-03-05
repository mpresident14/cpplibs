#ifndef PREZ_PARSERS_COMBINATOR_PARSERS_HPP
#define PREZ_PARSERS_COMBINATOR_PARSERS_HPP

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
}
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSERS_HPP
