#ifndef PREZ_PARSERS_COMBINATOR_MORE_TYPE_TRAITS_HPP
#define PREZ_PARSERS_COMBINATOR_MORE_TYPE_TRAITS_HPP

#include "src/misc/more_type_traits.hpp"
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

template <ParserPtr P>
struct pcomb_result {
  using type = typename std::decay_t<decltype(*std::declval<P>())>::result_type;
};

template <ParserPtr P>
using pcomb_result_t = typename pcomb_result<P>::type;

} // namespace detail
} // namespace pcomb
} // namespace prez
#endif // PREZ_PARSERS_COMBINATOR_TYPING_HPP
