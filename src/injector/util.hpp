#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace injector {

// Exposed so that clients don't have to explicitly order their parameters to use annotations
struct DefaultAnnotation {};

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

  static constexpr bool cstringEq(char const* a, char const* b) {
    return *a == *b && (*a == '\0' || cstringEq(a + 1, b + 1));
  }

  static constexpr bool isDefaultAnnotation(char const* annotationId) {
    return cstringEq(annotationId, getId<DefaultAnnotation>());
  }

}  // namespace detail
}  // namespace injector

#endif  // UTIL_HPP
