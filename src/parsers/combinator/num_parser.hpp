#ifndef PREZ_PARSERS_COMBINATOR_NUM_PARSER_HPP
#define PREZ_PARSERS_COMBINATOR_NUM_PARSER_HPP

#include "src/parsers/combinator/parser.hpp"

#include <charconv>
#include <string>

namespace prez {
namespace pcomb {
namespace detail {


template <typename Integer>
class IntegerParser : public Parser<Integer> {
public:
  IntegerParser(int base) : base_(base) {}

  ParseResult<Integer> tryParse(std::string_view input, const ParseOptions& options) override {
    Integer num;
    std::from_chars_result charsResult = std::from_chars(input.begin(), input.end(), num, base_);
    if (charsResult.ec == std::errc()) {
      std::string_view rest = input.substr(charsResult.ptr - input.begin());
      return {num, rest, {}, this->makeExeLog(options, input, rest, true)};
    }

    return {
        {},
        this->restIfCheckpted(input),
        this->getNameIfCheckpted(),
        this->makeExeLog(options, input, input, false)};
  }

protected:
  std::string getDefaultName() const override { return "Integer"; }

private:
  int base_;
};

// template <typename Decimal>
// class DecimalParser : public Parser<Decimal> {
// public:
//   DecimalParser(std::chars_format fmt) : fmt_(fmt) {}

//   ParseResult<Decimal> tryParse(std::string_view input) override {
//     Decimal num;
//     std::from_chars_result charsResult = std::from_chars(input.begin(), input.end(), num, fmt_);
//     if (charsResult.ec == std::errc()) {
//       return {{num}, {input.substr(charsResult.ptr - input.begin())}};
//     }

//     return {{}, input};
//   }

// private:
//   std::chars_format fmt_;
// };

} // namespace detail
} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_INT_PARSER_HPP
