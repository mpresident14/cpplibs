#ifndef INJECT_EXCEPTION_HPP
#define INJECT_EXCEPTION_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <prez/print_stuff.hpp>

namespace injector {
namespace detail {

  class InjectException : public std::exception {
  public:
    virtual const char* what() const noexcept override { return ""; }

    void addClass(std::string_view typeId) { injectionPath_.emplace_back(typeId); }

    friend std::ostream& operator<<(std::ostream& out, const InjectException& e) {
      if (e.injectionPath_.empty()) {
        return out;
      }

      out << "Type " << e.injectionPath_.front()
          << " is not bound and has no constructors for injection.";

      if (e.injectionPath_.size() == 1) {
        return out;
      }

      out << " Injection chain:\n\t";

      auto beginIter = std::prev(e.injectionPath_.crend());
      for (auto iter = e.injectionPath_.crbegin(); iter != beginIter; ++iter) {
        out << *iter << " -> ";
      }
      out << e.injectionPath_.front() << '.';
      return out;
    }

  private:
    // This is a backwards path (i.e. depN, ..., dep1, injectedClass)
    std::vector<std::string> injectionPath_;
  };

}  // namespace detail
}  // namespace injector

#endif  // INJECT_EXCEPTION_HPP
