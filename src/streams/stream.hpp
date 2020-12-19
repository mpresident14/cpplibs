// - Map*

// - Filter
// - Distinct
// - Limit

// interface Collector
// - ToVector
// - ToMap
// - ToSet
// - groupingBy
// - etc

// interface Reducer
// - Count
// - Sum
// - Max
// - etc

// - ForEach

#ifndef STREAM_HPP
#define STREAM_HPP


#include "src/streams/map_fn.hpp"
#include "src/streams/operations/distinct_op.hpp"
#include "src/streams/operations/filter_op.hpp"
#include "src/streams/operations/operation.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>


namespace prez {
namespace streams {
  using namespace detail;

  template <typename From, typename To, typename InitIter>
  class Stream;

  template <
      typename InitIter,
      typename From = std::remove_reference_t<decltype(*std::declval<InitIter>())>,
      typename To = std::remove_cv_t<From>>
  Stream<From, To, InitIter> streamFrom(InitIter begin, InitIter end);

  /**********
   * Stream *
   **********/
  template <typename From, typename To, typename InitIter>
  class Stream {
  private:
    template <typename From2, typename To2, typename Iter2>
    friend class Stream;

    using FriendStreamT = std::remove_reference_t<decltype(*std::declval<InitIter>())>;
    friend Stream<FriendStreamT, std::remove_cv_t<FriendStreamT>, InitIter> streamFrom<InitIter>(
        InitIter begin, InitIter end);

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
      return std::vector<To>(startIter, endIter);
    }

    template <typename Fn, typename NewType = std::invoke_result_t<Fn, To>>
    Stream<From, NewType, InitIter> map(Fn&& fn) {
      return Stream<From, NewType, InitIter>(
          begin_,
          end_,
          MapFn<From, NewType, InitIter>::fromFullMapper(
              [mapFn = std::move(mapFn_), ops = std::move(ops_), elemMapFn = std::forward<Fn>(fn)](
                  auto startRange, auto endRange) mutable {
                std::vector<To> outVec = mapFn.apply(startRange, endRange);
                vecIter<To> startIter = outVec.begin();
                vecIter<To> endIter = outVec.end();
                for (const auto& op : ops) {
                  op->apply(&startIter, &endIter);
                }
                return MapFn<To, NewType, vecIter<To>>::fromElemMapper(std::forward<Fn>(elemMapFn))
                    .apply(startIter, endIter);
              }),
          {});
    }


    template <typename Fn>
    requires std::predicate<Fn, To> Stream<From, To, InitIter>& filter(Fn&& fn) {
      ops_.push_back(std::make_unique<FilterOp<To>>(std::forward<Fn>(fn)));
      return *this;
    }

    // TODO: Overload with equality + comparable/hash functions
    // TODO: Requires equality + (comparable or hashable)
    template <typename To2 = To>
    requires std::totally_ordered<To2> Stream<From, To, InitIter>& distinct() {
      ops_.push_back(std::make_unique<DistinctOp<To>>());
      return *this;
    }

  private:
    Stream(
        InitIter begin,
        InitIter end,
        MapFn<From, To, InitIter>&& mapFn,
        std::vector<std::unique_ptr<Operation<To>>>&& ops)
        : begin_(begin), end_(end), mapFn_(std::move(mapFn)), ops_(std::move(ops)) {}


    InitIter begin_, end_;
    MapFn<From, To, InitIter> mapFn_;
    std::vector<std::unique_ptr<Operation<To>>> ops_;
  };


  template <typename InitIter, typename From, typename To>
  Stream<From, To, InitIter> streamFrom(InitIter begin, InitIter end) {
    return Stream<From, To, InitIter>(
        begin,
        end,
        MapFn<From, To, InitIter>::fromElemMapper([](const From& obj) { return obj; }),
        {});
  }

}  // namespace streams
}  // namespace prez

#endif  // STREAM_HPP
