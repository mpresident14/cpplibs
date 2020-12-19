#ifndef DISTINCT_OP_HPP
#define DISTINCT_OP_HPP

#include "src/streams/operations/operation.hpp"

#include <algorithm>
#include <functional>
#include <iostream>

namespace prez {
namespace streams {
  namespace detail {
    template <typename T>
    class DistinctOp : public Operation<T> {
    public:
      // template <typename Fn>
      // DistinctOp(Fn&& fn) : filterFn_(std::forward<Fn>(fn)) {}

      DistinctOp(){};  // = default;
      ~DistinctOp() = default;
      DistinctOp(const DistinctOp&) = delete;
      DistinctOp(DistinctOp&&) = default;
      DistinctOp& operator=(const DistinctOp&) = delete;
      DistinctOp& operator=(DistinctOp&&) = default;

      // Shout-out to https://stackoverflow.com/a/15761097!
      void apply(vecIter<T>* begin, vecIter<T>* end) override {
        std::vector<vecIter<T>> iters;
        iters.reserve(std::abs(std::distance(*begin, *end)));
        for (auto iter = *begin; iter != *end; ++iter) {
          iters.push_back(iter);
        }
        // Sort iters by the values to which they point.
        std::sort(iters.begin(), iters.end(), [](auto it1, auto it2) { return *it1 < *it2; });

        // Remove duplicate values and save the new endpoint.
        auto endDistinctIters = std::unique(
            iters.begin(), iters.end(), [](auto it1, auto it2) { return *it1 == *it2; });

        // Sort the remaining values by the iterators themselves (i.e. the indices) and save the new
        // endpoint.
        size_t numDistinct = std::distance(iters.begin(), endDistinctIters);
        std::sort(iters.begin(), endDistinctIters);
        endDistinctIters = iters.begin() + numDistinct;

        // Swap elements into their correct positions based on the iters vector.
        auto nextValIter = *begin;
        auto itersIter = iters.begin();
        for (auto valsIter = nextValIter; itersIter != endDistinctIters; ++valsIter) {
          if (valsIter == *itersIter) {
            std::iter_swap(valsIter, nextValIter);
            ++nextValIter;
            ++itersIter;
          }
        }

        *end = nextValIter;
      }

    private:
      // TODO: Should we really require this to be const T& or just T&? (same for MapFn and others)
      // std::function<bool(const T&)> filterFn_;
    };
  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // DISTINCT_OP_HPP
