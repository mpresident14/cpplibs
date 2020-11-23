#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace injector {
namespace detail {

  template <typename T>
  constexpr const char* getId() {
    return typeid(std::decay_t<T>).name();
  }

  template <typename... Args>
  void throwError(Args... msgParts) {
    std::ostringstream out;
    (..., (out << std::forward<Args>(msgParts)));
    throw std::runtime_error(out.str());
  }

}  // namespace detail
}  // namespace injector

#endif  // UTIL_HPP
