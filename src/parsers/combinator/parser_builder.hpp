#ifndef PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP
#define PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP

#include "src/parsers/combinator/parser.hpp"
#include "src/parsers/combinator/typing.hpp"

namespace prez {
namespace pcomb {

template <ParserPtr P>
class ParserBuilder;

template <ParserPtr P>
ParserBuilder<P> create(P&& parser);

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
  friend ParserBuilder create<P>(P&& parser);

  ParserBuilder(P&& parser) : parser_(std::move(parser)) {}
  ParserBuilder(P& parser) : parser_(parser) {}

  P parser_;
};


} // namespace pcomb
} // namespace prez

#endif // PREZ_PARSERS_COMBINATOR_PARSER_BUILDER_HPP
