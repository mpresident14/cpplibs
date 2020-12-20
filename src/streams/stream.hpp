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
#include "src/streams/util/ptr_iterator.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>


namespace prez {
namespace streams {
  using namespace detail;

  template <typename Iter>
  using iter_val_t = std::remove_reference_t<decltype(*std::declval<Iter>())>;

  template <typename To, typename InitIter>
  class Stream;

  template <typename InitIter, typename To = std::remove_cv_t<iter_val_t<InitIter>>>
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

    // TODO: Overload with equality + comparable/hash functions
    // TODO: Requires equality + (comparable or hashable)
    template <typename To2 = To>
    requires std::totally_ordered<To2> Stream<To, InitIter>& distinct() {
      ops_.push_back(std::make_unique<DistinctOp<To>>());
      return *this;
    }

  private:
    Stream(
        InitIter begin,
        InitIter end,
        MapFn<To, InitIter>&& mapFn,
        std::vector<std::unique_ptr<Operation<To>>>&& ops,
        bool isFirst = false)
        : begin_(begin),
          end_(end),
          mapFn_(std::move(mapFn)),
          ops_(std::move(ops)),
          isFirst_(isFirst) {}


    InitIter begin_, end_;
    MapFn<To, InitIter> mapFn_;
    std::vector<std::unique_ptr<Operation<To>>> ops_;
    bool isFirst_;
    // TODO: bool ordered
  };

  // TODO: The first vector should contain iterators pointing to the elements in the initial
  // container (to implement toRange, will need a special construct)
  template <typename InitIter, typename To>
  Stream<To, InitIter> streamFrom(InitIter begin, InitIter end) {
    return Stream<To, InitIter>(
        begin,
        end,
        MapFn<To, InitIter>::fromElemMapper([](const iter_val_t<InitIter>& obj) { return obj; }),
        {},
        /* isFirst */ true);
  }

}  // namespace streams
}  // namespace prez

#endif  // STREAM_HPP
