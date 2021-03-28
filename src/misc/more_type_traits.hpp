#ifndef PREZ_MISC_MORE_TYPE_TRAITS_HPP
#define PREZ_MISC_MORE_TYPE_TRAITS_HPP

#include <type_traits>
#include <utility>

#include <stddef.h>

namespace prez {
namespace misc {

// https://stackoverflow.com/a/28796458
template <typename T, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> {
  using type = std::tuple<Args*...>;
  static constexpr bool value = true;
};

template <typename T, template <typename...> class Ref>
constexpr bool is_specialization_v = is_specialization<std::decay_t<T>, Ref>::value;

template <typename T, template <typename...> class Ref>
using is_specialization_t = typename is_specialization<std::decay_t<T>, Ref>::type;

template <size_t I, typename T, template <typename...> class Ref>
using specialization_elem_t =
    typename std::remove_pointer_t<std::tuple_element_t<I, is_specialization_t<T, Ref>>>;

template <typename F>
struct num_args;

template <typename R, typename... Args>
struct num_args<R(Args...)> {
  static constexpr size_t value = sizeof...(Args);
};

template <typename F>
constexpr size_t num_args_v = num_args<F>::value;


} // namespace misc
} // namespace prez
#endif // PREZ_MISC_TYPING_HPP
