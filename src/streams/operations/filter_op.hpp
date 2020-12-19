#ifndef FILTER_OP_HPP
#define FILTER_OP_HPP

#include "src/streams/operations/operation.hpp"

#include <algorithm>
#include <functional>

namespace prez {
namespace streams {
  namespace detail {
    template <typename T>
    class FilterOp : public Operation<T> {
    public:
      template <typename Fn>
      FilterOp(Fn&& fn) : filterFn_(std::forward<Fn>(fn)) {}

      ~FilterOp() = default;
      FilterOp(const FilterOp&) = delete;
      FilterOp(FilterOp&&) = default;
      FilterOp& operator=(const FilterOp&) = delete;
      FilterOp& operator=(FilterOp&&) = default;

      void apply(vecIter<T>* begin, vecIter<T>* end) override {
        *end = std::remove_if(*begin, *end, std::not_fn(filterFn_));
      }

    private:
      // TODO: Should we really require this to be const T& or just T&? (same for MapFn and others)
      std::function<bool(const T&)> filterFn_;
    };
  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // FILTER_OP_HPP
