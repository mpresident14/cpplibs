#include <chrono>
#include <cstddef>
#include <type_traits>

namespace misc {

template <typename Unit = std::chrono::microseconds, typename F, typename... Args>
size_t timeit(size_t n, F&& f, Args&&... args) {
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < n; ++i) {
    f(std::forward<Args>(args)...);
  }
  auto stop = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<Unit>(stop - start).count();
}


template <typename Unit = std::chrono::microseconds, typename F, typename... Args>
decltype(auto) timeitReturn(size_t& result, size_t n, F&& f, Args&&... args) {
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < n - 1; ++i) {
    f(std::forward<Args>(args)...);
  }
  auto ret = f(std::forward<Args>(args)...);
  auto stop = std::chrono::high_resolution_clock::now();
  result = std::chrono::duration_cast<Unit>(stop - start).count();
  return ret;
}

}  // namespace misc
