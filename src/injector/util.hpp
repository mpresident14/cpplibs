#ifndef PREZ_INJECTOR_UTIL_HPP
#define PREZ_INJECTOR_UTIL_HPP

#include <sstream>
#include <string>
#include <type_traits>

namespace prez {
namespace injector {

// Exposed so that clients don't have to explicitly order their parameters to
// use annotations
struct Unannotated {};

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

void streamAnnotated(std::ostream& out, const char* annotationId) {
  if (!cstringEq(annotationId, getId<Unannotated>())) {
    out << " (annotated with " << annotationId << ')';
  }
}

template <typename Annotation>
void streamAnnotated(std::ostream& out) {
  if constexpr (!std::is_same_v<Annotation, Unannotated>) {
    out << " (annotated with " << getId<Annotation>() << ')';
  }
}

} // namespace detail
} // namespace injector
} // namespace prez

#endif // UTIL_HPP
