#ifndef PREZ_STREAMS_OPERATIONS_FOR_EACH_OP_HPP
#define PREZ_STREAMS_OPERATIONS_FOR_EACH_OP_HPP


#include "src/streams/operations/operation.hpp"

#include <algorithm>
#include <functional>

namespace prez {
namespace streams {
namespace detail {
template <typename T, typename Consumer>
class ForEachOp : public Operation<T> {
public:
  ForEachOp(Consumer&& consumer) : consumer_(std::forward<Consumer>(consumer)) {}

  ~ForEachOp() = default;
  ForEachOp(const ForEachOp&) = delete;
  ForEachOp(ForEachOp&&) = default;
  ForEachOp& operator=(const ForEachOp&) = delete;
  ForEachOp& operator=(ForEachOp&&) = default;

  void apply(vecIter<T>* begin, vecIter<T>* end) override {
    std::for_each(*begin, *end, std::ref(consumer_));
  }

private:
  Consumer consumer_;
};
} // namespace detail
} // namespace streams
} // namespace prez

#endif // PREZ_STREAMS_OPERATIONS_FOR_EACH_OP_HPP
