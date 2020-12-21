#ifndef DISTINCT_OP_HPP
#define DISTINCT_OP_HPP

#include "src/streams/operations/operation.hpp"
#include "src/streams/typing.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_set>

namespace prez {
namespace streams {
  namespace detail {
    template <typename T>
    class DistinctOp : public Operation<T> {
    public:
      // template <typename Fn>
      // DistinctOp(Fn&& fn) : filterFn_(std::forward<Fn>(fn)) {}

      DistinctOp() = default;
      ~DistinctOp() = default;
      DistinctOp(const DistinctOp&) = delete;
      DistinctOp(DistinctOp&&) = default;
      DistinctOp& operator=(const DistinctOp&) = delete;
      DistinctOp& operator=(DistinctOp&&) = default;

      void apply(vecIter<T>* begin, vecIter<T>* end) override { applyImpl(begin, end); }

      // Shout-out to https://stackoverflow.com/a/15761097 to avoid overhead of std::set!
      template <typename Unwrapped = remove_ref_wrap_t<T>>
      requires(DistinctableBySort<Unwrapped> && !DistinctableByHash<Unwrapped>) void applyImpl(
          vecIter<T>* begin, vecIter<T>* end) {
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

        *end = distinctify(*begin, iters.begin(), endDistinctIters);
      }


      template <typename Unwrapped = remove_ref_wrap_t<T>>
      requires DistinctableByHash<Unwrapped> void applyImpl(vecIter<T>* begin, vecIter<T>* end) {
        struct EqFn {
          bool operator()(const vecIter<T>& it1, const vecIter<T>& it2) const {
            return *it1 == *it2;
          }
        };
        struct HashFn {
          size_t operator()(const vecIter<T>& it) const { return std::hash<Unwrapped>{}(*it); }
        };

        std::unordered_set<vecIter<T>, HashFn, EqFn> seen;
        std::vector<vecIter<T>> iters;
        iters.reserve(std::abs(std::distance(*begin, *end)));
        for (auto iter = *begin; iter != *end; ++iter) {
          if (seen.insert(iter).second) {
            iters.push_back(iter);
          }
        }

        *end = distinctify(*begin, iters.begin(), iters.end());
      }

    private:
      vecIter<T> distinctify(
          vecIter<T> nextValIter,
          vecIter<vecIter<T>> nextIterIter,
          vecIter<vecIter<T>> endDistinctIters) {
        // Swap elements into their correct positions based on the iters vector.
        for (auto valsIter = nextValIter; nextIterIter != endDistinctIters; ++valsIter) {
          if (valsIter == *nextIterIter) {
            std::iter_swap(valsIter, nextValIter);
            ++nextValIter;
            ++nextIterIter;
          }
        }
        return nextValIter;
      }
    };
  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // DISTINCT_OP_HPP
