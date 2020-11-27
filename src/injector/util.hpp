#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <string>
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
  std::string strCat(Args... msgParts) {
    std::ostringstream out;
    (..., (out << std::forward<Args>(msgParts)));
    return out.str();
  }

  constexpr bool cstringEq(char const* a, char const* b) {
    return *a == *b && (*a == '\0' || cstringEq(a + 1, b + 1));
  }

  constexpr bool isDefaultAnnotation(char const* annotationId) {
    return cstringEq(annotationId, getId<DefaultAnnotation>());
  }

  template <typename Annotation>
  void streamNonDefault(std::ostream& out) {
    if constexpr (std::is_same_v<Annotation, DefaultAnnotation>) {
      out << " (annotated with " << getId<Annotation>() << ')';
    }
  }

  void streamNonDefault(std::ostream& out, const char* annotationId) {
    if (!isDefaultAnnotation(annotationId)) {
      out << " (annotated with " << annotationId << ')';
    }
  }

}  // namespace detail
}  // namespace injector

#endif  // UTIL_HPP
