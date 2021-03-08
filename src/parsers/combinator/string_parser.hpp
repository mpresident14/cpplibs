#ifndef PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP

#include "src/parsers/combinator/parser.hpp"

#include <string>

namespace prez {
namespace pcomb {
namespace detail {


class StringParser : public Parser<std::string> {
public:
  StringParser(std::string_view sv) : str_(sv) {}

  ParseResult<std::string> tryParse(std::string_view input) override {
    if (input.starts_with(str_)) {
      return {true, str_, input.substr(str_.size())};
    }
    return {false, std::vector<std::string>{this->nameForError_}, input};
  }

private:
  std::string str_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
