#ifndef PREZ_MISC_TYPING_HPP
#define PREZ_MISC_TYPING_HPP


#include <type_traits>

namespace prez {
namespace misc {

namespace detail {

// https://stackoverflow.com/a/28796458
template <typename T, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

} // namespace detail

template <typename T, template <typename...> class Ref>
static constexpr bool is_specialization_v = detail::is_specialization<std::decay_t<T>, Ref>::value;

} // namespace misc
} // namespace prez
#endif // PREZ_MISC_TYPING_HPP
