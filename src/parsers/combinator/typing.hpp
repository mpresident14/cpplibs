#ifndef PREZ_PARSERS_COMBINATOR_TYPING_HPP
#define PREZ_PARSERS_COMBINATOR_TYPING_HPP

#include "src/misc/typing.hpp"
#include "src/parsers/combinator/parser.hpp"

#include <memory>
#include <type_traits>

namespace prez {
namespace pcomb {
namespace detail {

template <typename T, template <typename...> class Ptr>
concept ParserPtrImpl = std::derived_from<
    misc::specialization_elem_t<0, T, Ptr>,
    Parser<typename misc::specialization_elem_t<0, T, Ptr>::result_type>>;

template <typename>
struct parser_ptr : std::false_type {};

template <typename T>
requires ParserPtrImpl<T, std::shared_ptr> struct parser_ptr<T> : std::true_type {};

template <typename T>
requires ParserPtrImpl<T, std::unique_ptr> struct parser_ptr<T> : std::true_type {};


template <typename T>
concept ParserPtr = parser_ptr<T>::value;

} // namespace detail
} // namespace pcomb
} // namespace prez
#endif // PREZ_PARSERS_COMBINATOR_TYPING_HPP
