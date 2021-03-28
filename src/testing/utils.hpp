#ifndef PREZ_TESTING_UTILS_HPP
#define PREZ_TESTING_UTILS_HPP


#include <iostream>
#include <sstream>
#include <type_traits>

namespace prez {
namespace testing {
namespace detail {
class BufCaptor {
public:
  BufCaptor(std::ostream& coutOrCerr, std::string& captor)
      : coutOrCerr_(coutOrCerr), captor_(captor), coutBuf_(std::cout.rdbuf()) {
    coutOrCerr.rdbuf(out_.rdbuf());
  }
  // RAII: Destructor is responsible for releasing resources.
  ~BufCaptor() {
    captor_ = out_.str();
    coutOrCerr_.rdbuf(coutBuf_);
  }

private:
  std::ostream& coutOrCerr_;
  std::string& captor_;
  std::streambuf* coutBuf_;
  std::ostringstream out_;
};
} // namespace detail

template <typename F>
requires std::is_void_v<std::invoke_result_t<F()>> void captureStdOut(std::string& captor, F&& fn) {
  detail::BufCaptor setBuf(std::cout, captor);
  fn();
}

template <typename F>
requires(!std::is_void_v<std::invoke_result_t<F()>>) auto captureStdOut(
    std::string& captor, F&& fn) {
  detail::BufCaptor setBuf(std::cout, captor);
  return fn();
}

} // namespace testing
} // namespace prez

#endif // PREZ_TESTING_UTILS_HPP
