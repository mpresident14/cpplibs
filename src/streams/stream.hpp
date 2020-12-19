// Stream<To, From, Start> implements BasePartialStream<To, Start>
// unique_ptr<BasePartialStream<From, Start>> prev;
// Ctor: : void(iter<T> begin, iter<T> end)

// interface Step: <T> optional<linkedlist/vector<To>> process(InitIter begin, InitIter end)
// either updates iters or returns new container. Impl: Call prev to get to iter<From>, then
// do operation From -> To
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


// Stream(vector<int>).map(i -> str(i)).filter(len > 3).collect(toList());

// BasePartialStream<int, int> ->  Stream<string, int, int> (has BasePartialStream<int, int>)
// ->  Stream<string, string, int> (has BasePartialStream<string, int>)

// Optimization: Any operation that doesn't change the type can just append an Operation to a vector
// within the given Stream object

#ifndef STREAM_HPP
#define STREAM_HPP


#include "src/streams/map_fn.hpp"
#include "src/streams/operations/distinct_op.hpp"
#include "src/streams/operations/filter_op.hpp"
#include "src/streams/operations/operation.hpp"
#include "src/streams/typing.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>


namespace prez {
namespace streams {
  using namespace detail;

  template <typename From, typename To, typename PrevTuple, typename InitIter>
  class Stream;

  template <
      typename InitIter,
      typename T = std::remove_reference_t<decltype(*std::declval<InitIter>())>>
  Stream<T, T, std::tuple<>, InitIter> streamFrom(InitIter begin, InitIter end);

  /**********
   * Stream *
   **********/
  template <typename From, typename To, typename PrevTuple, typename InitIter>
  class Stream {
  private:
    // Initial node will use user-supplied iterator. Subsequent nodes will use the iterator of the
    // internal container (a vector).
    using ThisMapFn = MapFn<From, To, cond_tuple_empty_t<PrevTuple, InitIter, vecIter<From>>>;

    using PrevSNPtr =
        std::unique_ptr<Stream<last_arg_t<PrevTuple>, From, subtuple1_t<PrevTuple>, InitIter>>;

    template <typename From2, typename To2, typename PrevTuple2, typename Iter2>
    friend class Stream;

    using FriendStreamT = std::remove_reference_t<decltype(*std::declval<InitIter>())>;
    friend Stream<FriendStreamT, FriendStreamT, std::tuple<>, InitIter> streamFrom<InitIter>(
        InitIter begin, InitIter end);

  public:
    Stream(const Stream&) = delete;
    Stream(Stream&&) = default;
    Stream& operator=(const Stream&) = delete;
    Stream& operator=(Stream&&) = default;

    std::vector<To> toVector() { return combineAll().toVectorImpl(); }

    /* Invalidates the Stream upon which it is called. */
    template <typename Fn, typename NewType = std::invoke_result_t<Fn, To>>
    Stream<To, NewType, tup_append_t<PrevTuple, From>, InitIter> map(Fn&& fn) {
      return Stream<To, NewType, tup_append_t<PrevTuple, From>, InitIter>(
          begin_,
          end_,
          std::make_unique<Stream<From, To, PrevTuple, InitIter>>(std::move(*this)),
          MapFn<To, NewType, vecIter<To>>::fromElemMapper(std::forward<Fn>(fn)),
          {});
    }

    template <
        typename Fn,
        typename = std::enable_if_t<std::is_convertible_v<bool, std::invoke_result_t<Fn, To>>>>
    Stream<From, To, PrevTuple, InitIter>& filter(Fn&& fn) {
      ops_.push_back(std::make_unique<FilterOp<To>>(std::forward<Fn>(fn)));
      return *this;
    }

    Stream<From, To, PrevTuple, InitIter>& distinct() {
      ops_.push_back(std::make_unique<DistinctOp<To>>());
      return *this;
    }

  private:
    Stream(
        InitIter begin,
        InitIter end,
        PrevSNPtr&& prev,
        ThisMapFn&& mapFn,
        std::vector<std::unique_ptr<Operation<To>>>&& ops)
        : begin_(begin),
          end_(end),
          prev_(std::move(prev)),
          mapFn_(std::move(mapFn)),
          ops_(std::move(ops)) {}


    std::vector<To> toVectorImpl() {
      std::vector<To> toVec = mapFn_.apply(begin_, end_);
      vecIter<To> startIter = toVec.begin();
      vecIter<To> endIter = toVec.end();
      for (const auto& op : ops_) {
        op->apply(&startIter, &endIter);
      }
      return std::vector<To>(startIter, endIter);
    }


    // SFINAE has to be applied on a function template parameter
    template <typename PrevTuple2 = PrevTuple>
    std::enable_if_t<
        (std::tuple_size_v<PrevTuple2> > 1),
        Stream<first_arg_t<PrevTuple2>, To, std::tuple<>, InitIter>>
    combineAll() {
      // Cannot call on the initial Stream (should not reach this due to SFINAE)
      // TODO: Remove after testing
      // if (!prev_) {
      //   throw std::runtime_error("Stream::combineAll: nullptr prev_");
      // }
      return buildCombinedNode().combineAll();
    }

    template <typename PrevTuple2 = PrevTuple>
    std::enable_if_t<
        std::tuple_size_v<PrevTuple2> == 1,
        Stream<first_arg_t<PrevTuple2>, To, std::tuple<>, InitIter>>
    combineAll() {
      // Cannot call on the initial Stream (should not reach this due to SFINAE)
      // TODO: Remove after testing
      // if (!prev_) {
      //   throw std::runtime_error("Stream::combineAll: nullptr prev_");
      // }
      return buildCombinedNode();
    }

    template <typename PrevTuple2 = PrevTuple>
    std::enable_if_t<std::tuple_size_v<PrevTuple2> == 0, Stream<From, To, std::tuple<>, InitIter>>&
    combineAll() {
      return *this;
    }

    template <typename PrevTuple2 = PrevTuple, typename NewPrevTuple = subtuple1_t<PrevTuple>>
    std::enable_if_t<
        std::tuple_size_v<PrevTuple2> != 0,
        Stream<last_arg_t<PrevTuple>, To, NewPrevTuple, InitIter>>
    buildCombinedNode() {
      using PrevFrom = first_arg_t<PrevTuple>;

      return Stream<last_arg_t<PrevTuple>, To, NewPrevTuple, InitIter>(
          begin_,
          end_,
          std::move(prev_->prev_),
          // The initial MapFn needs to take in the iterator supplied by the user when the initial
          // Stream was created.
          MapFn<PrevFrom, To, cond_tuple_empty_t<NewPrevTuple, InitIter, vecIter<PrevFrom>>>::
              fromFullMapper([prev = std::move(prev_), mapFn = std::move(mapFn_)](
                                 auto startRange, auto endRange) mutable {
                std::vector<From> outVec = prev->mapFn_.apply(startRange, endRange);
                vecIter<From> startIter = outVec.begin();
                vecIter<From> endIter = outVec.end();
                for (const auto& op : prev->ops_) {
                  op->apply(&startIter, &endIter);
                }
                return mapFn.apply(startIter, endIter);
              }),
          std::move(ops_));
    }


    InitIter begin_, end_;
    PrevSNPtr prev_;
    ThisMapFn mapFn_;
    std::vector<std::unique_ptr<Operation<To>>> ops_;
  };


  template <typename InitIter, typename T>
  Stream<T, T, std::tuple<>, InitIter> streamFrom(InitIter begin, InitIter end) {
    return Stream<T, T, std::tuple<>, InitIter>(
        begin,
        end,
        nullptr,
        MapFn<T, T, InitIter>::fromElemMapper([](const T& obj) { return obj; }),
        {});
  }

}  // namespace streams
}  // namespace prez

#endif  // STREAM_HPP
