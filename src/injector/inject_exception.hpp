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

  class InjectException : public std::runtime_error {
  public:
    InjectException(const char* msg) : std::runtime_error(msg) {}
    InjectException(std::string msg) : std::runtime_error(msg) {}
    virtual ~InjectException() noexcept {}

    void addClass(const char* typeId, const char* annotationId) {
      injectionPath_.emplace_back(typeId, annotationId);
    }

    friend std::ostream& operator<<(std::ostream& out, const InjectException& e) {
      if (e.injectionPath_.empty()) {
        return out;
      }


      out << e.what();

      // out << "Type " << firstPair.first;
      // InjectException::streamNonDefault(out, firstPair.second);
      // out << " is not bound and has no constructors for injection.";

      if (e.injectionPath_.size() == 1) {
        return out;
      }

      out << " Injection chain:\n\t";

      auto beginIter = std::prev(e.injectionPath_.crend());
      for (auto iter = e.injectionPath_.crbegin(); iter != beginIter; ++iter) {
        out << iter->first;
        streamNonDefault(out, iter->second);
        out << " -> ";
      }

      const auto& firstPair = e.injectionPath_.front();
      out << firstPair.first;
      streamNonDefault(out, firstPair.second);
      out << '.';
      return out;
    }

  private:
    // This is a backwards path (i.e. depN, ..., dep1, injectedClass), so we print it in reverse.
    std::vector<std::pair<const char*, const char*>> injectionPath_;
  };

}  // namespace detail
}  // namespace injector

#endif  // INJECT_EXCEPTION_HPP
