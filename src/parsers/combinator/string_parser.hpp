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

  ParseResult<std::string> tryParse(std::string_view input, const ParseOptions& options) override {
    if (input.starts_with(str_)) {
      std::string_view rest = input.substr(str_.size());
      return {str_, rest, {}, this->makeExeLog(options, input, rest, true)};
    }
    return {
        {},
        this->restIfCheckpted(input),
        this->getNameIfCheckpted(),
        this->makeExeLog(options, input, input, false)};
  }

protected:
  std::string getDefaultName() const override { return str_; }

private:
  std::string str_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
