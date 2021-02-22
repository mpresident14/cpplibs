#ifndef PREZ_STREAMS_STREAM_HPP
#define PREZ_STREAMS_STREAM_HPP

#include "src/streams/map_fn.hpp"
#include "src/streams/operations/distinct_op.hpp"
#include "src/streams/operations/filter_op.hpp"
#include "src/streams/operations/operation.hpp"
#include "src/streams/typing.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace prez {
namespace streams {
using namespace detail;

template <typename To, typename InitIter>
class Stream;

template <
    typename InitIter,
    typename To = std::reference_wrapper<std::add_const_t<iter_val_t<InitIter>>>>
Stream<To, InitIter> streamFrom(InitIter begin, InitIter end);

template <typename To, typename InitIter>
class Stream {
private:
  template <typename To2, typename Iter2>
  friend class Stream;

  friend Stream<To, InitIter> streamFrom<InitIter, To>(InitIter begin, InitIter end);

public:
  Stream(const Stream&) = delete;
  Stream(Stream&&) = default;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = default;

  std::vector<To> toVector() {
    std::vector<To> toVec = mapFn_.apply(begin_, end_);
    vecIter<To> startIter = toVec.begin();
    vecIter<To> endIter = toVec.end();
    for (const auto& op : ops_) {
      op->apply(&startIter, &endIter);
    }
    return std::vector<To>(std::make_move_iterator(startIter), std::make_move_iterator(endIter));
  }

  /*
   * Like toVector, but makes a copy. Mainly to be used on a non-mapped stream
   * to transform the reference_wrapper implicitly for cheaply copied objects.
   */
  std::vector<remove_ref_wrap_t<To>> toVectorCopy() {
    std::vector<To> toVec = mapFn_.apply(begin_, end_);
    vecIter<To> startIter = toVec.begin();
    vecIter<To> endIter = toVec.end();
    for (const auto& op : ops_) {
      op->apply(&startIter, &endIter);
    }

    return std::vector<remove_ref_wrap_t<To>>(startIter, endIter);
  }

  template <typename Fn, typename NewType = std::invoke_result_t<Fn, To>>
  Stream<NewType, InitIter> map(Fn&& fn) {
    return Stream<NewType, InitIter>(
        begin_,
        end_,
        MapFn<NewType, InitIter>::fromFullMapper(
            [mapFn = std::move(mapFn_), ops = std::move(ops_), elemMapFn = std::forward<Fn>(fn)](
                auto startRange, auto endRange) mutable {
              std::vector<To> outVec = mapFn.apply(startRange, endRange);
              vecIter<To> startIter = outVec.begin();
              vecIter<To> endIter = outVec.end();
              for (const auto& op : ops) {
                op->apply(&startIter, &endIter);
              }
              return MapFn<NewType, vecIter<To>>::fromElemMapper(std::forward<Fn>(elemMapFn))
                  .apply(startIter, endIter);
            }),
        {});
  }

  template <typename Fn>
  requires std::predicate<Fn, To> Stream<To, InitIter>& filter(Fn&& fn) {
    ops_.push_back(std::make_unique<FilterOp<To>>(std::forward<Fn>(fn)));
    return *this;
  }

  /* Keeps only the first occurence if ordered container. Otherwise, any
   * occurrence. */
  template <typename Unwrapped = remove_ref_wrap_t<To>>
  requires(DistinctableByHash<Unwrapped>) Stream<To, InitIter>& distinct() {
    ops_.push_back(std::make_unique<DistinctHashOp<To>>());
    return *this;
  }

  template <typename Unwrapped = remove_ref_wrap_t<To>>
  requires(DistinctableBySort<Unwrapped> && !DistinctableByHash<Unwrapped>)
      Stream<To, InitIter>& distinct() {
    ops_.push_back(std::make_unique<DistinctSortOp<To>>());
    return *this;
  }

  template <typename LTFn, typename Unwrapped = remove_ref_wrap_t<To>>
  requires std::predicate<LTFn, const Unwrapped&, const Unwrapped&> Stream<To, InitIter>&
  distinct(LTFn&& ltFn) {
    ops_.push_back(std::make_unique<DistinctSortOp<To>>(std::forward<LTFn>(ltFn)));
    return *this;
  }

  template <typename HashFn, typename EqFn, typename Unwrapped = remove_ref_wrap_t<To>>
  requires(std::is_convertible_v<std::invoke_result_t<HashFn, const Unwrapped&>, size_t>&&
               std::predicate<EqFn, const Unwrapped&, const Unwrapped&>)
      Stream<To, InitIter>& distinct(HashFn&& hashFn, EqFn&& eqFn) {
    ops_.push_back(std::make_unique<DistinctHashOp<To>>(
        std::forward<HashFn>(hashFn), std::forward<EqFn>(eqFn)));
    return *this;
  }

  /* requires Unwrapped to be addable and have a default constructor (for additive identity). */
  template <typename Unwrapped = remove_ref_wrap_t<To>>
  decltype(std::declval<Unwrapped>() + std::declval<Unwrapped>()) sum() {
    std::vector<To> vec = toVector();
    return std::reduce(vec.cbegin(), vec.cend(), Unwrapped{}, std::plus<Unwrapped>());
  };

private:
  Stream(
      InitIter begin,
      InitIter end,
      MapFn<To, InitIter>&& mapFn,
      std::vector<std::unique_ptr<Operation<To>>>&& ops)
      : begin_(begin), end_(end), mapFn_(std::move(mapFn)), ops_(std::move(ops)) {}

  InitIter begin_, end_;
  MapFn<To, InitIter> mapFn_;
  std::vector<std::unique_ptr<Operation<To>>> ops_;
  // TODO: bool ordered
};

template <typename InitIter, typename To>
Stream<To, InitIter> streamFrom(InitIter begin, InitIter end) {
  return Stream<To, InitIter>(
      begin,
      end,
      MapFn<To, InitIter>::fromElemMapper(
          [](const iter_val_t<InitIter>& obj) { return std::ref(obj); }),
      {});
}

} // namespace streams
} // namespace prez

#endif // STREAM_HPP
