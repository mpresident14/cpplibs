#ifndef PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP
#define PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP

#include "src/parsers/combinator/more_type_traits.hpp"
#include "src/parsers/combinator/parser.hpp"

namespace prez {
namespace pcomb {

template <ParserPtr P>
class ParserBuilder;

template <ParserPtr P>
ParserBuilder<std::remove_reference_t<P>> builder(P&& parser);

template <ParserPtr P>
class ParserBuilder {
public:
  ParserBuilder& withErrCheckpt() {
    parser_->setErrCheckpt();
    return *this;
  }

  ParserBuilder& withName(std::string_view name) {
    parser_->setName(name);
    return *this;
  }

  // TODO: Don't allow this to be called twice.
  P build() { return std::move(parser_); }

private:
  template <ParserPtr P2>
  friend ParserBuilder<std::remove_reference_t<P2>> builder(P2&& parser);

  ParserBuilder(P&& parser) : parser_(std::move(parser)) {}
  ParserBuilder(P& parser) : parser_(parser) {}

  P parser_;
};


} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP
