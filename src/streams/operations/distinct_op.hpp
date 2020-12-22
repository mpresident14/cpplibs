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


    template <typename T>
    class DistinctSortOp : public Operation<T> {
    public:
      DistinctSortOp()
          : ltFn_([](auto it1, auto it2) { return *it1 < *it2; }),
            eqFn_([](auto it1, auto it2) { return *it1 == *it2; }) {}

      template <typename LTFn>
      DistinctSortOp(LTFn&& ltFn)
          : ltFn_(
              [ltFn = std::forward<LTFn>(ltFn)](auto it1, auto it2) { return ltFn(*it1, *it2); }),
            eqFn_([this](auto it1, auto it2) { return !ltFn_(it1, it2) && !ltFn_(it2, it1); }) {}

      ~DistinctSortOp() = default;
      DistinctSortOp(const DistinctSortOp&) = delete;
      DistinctSortOp(DistinctSortOp&&) = default;
      DistinctSortOp& operator=(const DistinctSortOp&) = delete;
      DistinctSortOp& operator=(DistinctSortOp&&) = default;

      void apply(vecIter<T>* begin, vecIter<T>* end) override {
        std::vector<vecIter<T>> iters;
        iters.reserve(std::abs(std::distance(*begin, *end)));
        for (auto iter = *begin; iter != *end; ++iter) {
          iters.push_back(iter);
        }

        // Sort iters by the values to which they point.
        std::sort(iters.begin(), iters.end(), ltFn_);

        // Remove duplicate values and save the new endpoint.
        auto endDistinctIters = std::unique(iters.begin(), iters.end(), eqFn_);

        // Sort the remaining values by the iterators themselves (i.e. the indices) and save the new
        // endpoint.
        size_t numDistinct = std::distance(iters.begin(), endDistinctIters);
        std::sort(iters.begin(), endDistinctIters);
        endDistinctIters = iters.begin() + numDistinct;

        *end = distinctify<T>(*begin, iters.begin(), endDistinctIters);
      }

    private:
      std::function<bool(vecIter<T> it1, vecIter<T> it2)> ltFn_;
      std::function<bool(vecIter<T> it1, vecIter<T> it2)> eqFn_;
    };


    template <typename T>
    class DistinctHashOp : public Operation<T> {
    public:
      DistinctHashOp()
          : DistinctHashOp(
              std::hash<remove_ref_wrap_t<T>>{}, std::equal_to<remove_ref_wrap_t<T>>{}) {}


      template <typename HashFn, typename EqFn>
      DistinctHashOp(HashFn&& hashFn, EqFn&& eqFn)
          : hashFn_([hashFn = std::forward<HashFn>(hashFn)](auto it) { return hashFn(*it); }),
            eqFn_([eqFn = std::forward<EqFn>(eqFn)](auto it1, auto it2) {
              return eqFn(*it1, *it2);
            }) {}

      ~DistinctHashOp() = default;
      DistinctHashOp(const DistinctHashOp&) = delete;
      DistinctHashOp(DistinctHashOp&&) = default;
      DistinctHashOp& operator=(const DistinctHashOp&) = delete;
      DistinctHashOp& operator=(DistinctHashOp&&) = default;

      void apply(vecIter<T>* begin, vecIter<T>* end) override {
        size_t numElems = std::abs(std::distance(*begin, *end));
        // Capture the hash and eq functions by reference in a lambda so that the set does not have
        // to make a copy.
        std::unordered_set<vecIter<T>, decltype(hashFn_), decltype(eqFn_)> seen(
            numElems,
            [this](auto it) { return hashFn_(it); },
            [this](auto it1, auto it2) { return eqFn_(it1, it2); });
        std::vector<vecIter<T>> iters;
        iters.reserve(numElems);
        for (auto iter = *begin; iter != *end; ++iter) {
          if (seen.insert(iter).second) {
            iters.push_back(iter);
          }
        }

        *end = distinctify<T>(*begin, iters.begin(), iters.end());
      }

    private:
      std::function<size_t(vecIter<T>)> hashFn_;
      std::function<bool(vecIter<T>, vecIter<T>)> eqFn_;
    };

  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // DISTINCT_OP_HPP
