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
      return {str_, input.substr(str_.size()), {}, this->makeExeLog(options, input, true)};
    }
    return {
        {},
        this->restIfCheckpted(input),
        this->getNameForFailure(),
        this->makeExeLog(options, input, false)};
  }

protected:
  std::string getDefaultName() const override {
    return std::string(1, '"').append(str_).append(1, '"');
  }

private:
  std::string str_;
};

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_STRING_PARSER_HPP
