#ifndef PREZ_STREAMS_OPERATIONS_FILTER_OP_HPP
#define PREZ_STREAMS_OPERATIONS_FILTER_OP_HPP

#include "src/streams/operations/operation.hpp"

#include <algorithm>
#include <functional>

namespace prez {
namespace streams {
namespace detail {
template <typename T, typename Fn, typename Wrapper = T>
class FilterOp : public Operation<Wrapper> {
public:
  FilterOp(Fn&& fn) : filterFn_(std::forward<Fn>(fn)) {}

  ~FilterOp() = default;
  FilterOp(const FilterOp&) = delete;
  FilterOp(FilterOp&&) = default;
  FilterOp& operator=(const FilterOp&) = delete;
  FilterOp& operator=(FilterOp&&) = default;

  void apply(vecIter<Wrapper>* begin, vecIter<Wrapper>* end) override {
    *end = std::remove_if(*begin, *end, std::not_fn(std::ref(filterFn_)));
  }

private:
  Fn filterFn_;
};
} // namespace detail
} // namespace streams
} // namespace prez

#endif // FILTER_OP_HPP
