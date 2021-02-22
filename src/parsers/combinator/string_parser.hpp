#ifndef PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP

#include "src/parsers/combinator/parser.hpp"

#include <string>

namespace prez {
namespace pcomb {


class StringParser : public Parser<std::string> {
public:
  StringParser(std::string_view sv) : str_(sv) {}

  ParseResult tryParse(std::string_view input) {
    if (input.starts_with(str_)) {
      return {{str_}, input.substr(str_.size())};
    }
    return {{}, input};
  }


private:
  std::string str_;
};

} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
