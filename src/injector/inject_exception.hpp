#ifndef INJECT_EXCEPTION_HPP
#define INJECT_EXCEPTION_HPP

#include "src/injector/util.hpp"

#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace injector {
namespace detail {

  class InjectException : public std::exception {
  public:
    virtual const char* what() const noexcept override { return ""; }

    void addClass(const char* typeId, const char* annotationId) {
      injectionPath_.emplace_back(
          typeId, isDefaultAnnotation(annotationId) ? nullptr : annotationId);
    }

    friend std::ostream& operator<<(std::ostream& out, const InjectException& e) {
      if (e.injectionPath_.empty()) {
        return out;
      }

      const auto& firstPair = e.injectionPath_.front();

      out << "Type " << firstPair.first;
      InjectException::streamNonDefault(out, firstPair.second);
      out << " is not bound and has no constructors for injection.";

      if (e.injectionPath_.size() == 1) {
        return out;
      }

      out << " Injection chain:\n\t";

      auto beginIter = std::prev(e.injectionPath_.crend());
      for (auto iter = e.injectionPath_.crbegin(); iter != beginIter; ++iter) {
        out << iter->first;
        InjectException::streamNonDefault(out, iter->second);
        out << " -> ";
      }
      out << firstPair.first;
      InjectException::streamNonDefault(out, firstPair.second);
      out << '.';
      return out;
    }

  private:
    static void streamNonDefault(std::ostream& out, const char* annotationId) {
      if (annotationId) {
        out << " (annotated with " << annotationId << ')';
      }
    }
    // This is a backwards path (i.e. depN, ..., dep1, injectedClass), so we print it in reverse.
    std::vector<std::pair<const char*, const char*>> injectionPath_;
  };

}  // namespace detail
}  // namespace injector

#endif  // INJECT_EXCEPTION_HPP
