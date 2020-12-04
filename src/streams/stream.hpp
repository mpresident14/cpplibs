// Stream<To, From, Start> implements BasePartialStream<To, Start>
// unique_ptr<BasePartialStream<From, Start>> prev;
// Ctor: : void(iter<T> begin, iter<T> end)

// interface Step: <T> optional<linkedlist/vector<To>> process(Iter begin, Iter end)
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

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Iter>
class Operation {
public:
  virtual ~Operation() = default;
  virtual void apply(Iter* begin, Iter* end) = 0;
};

template <typename T, template <typename> class Iter>
class FilterOp : public Operation<Iter<T>> {
public:
  template <typename Fn>
  FilterOp(Fn&& filterFn) : filterFn_(std::forward<Fn>(filterFn)) {}

  virtual void apply(Iter<T>* begin, Iter<T>* end) = 0;

private:
  std::function<void(const T&)> filterFn_;
};


template <typename InitIter, typename End>
class BasePartialStream {
public:
  virtual ~BasePartialStream() = default;
  virtual std::vector<End> process(InitIter* begin, InitIter* end) = 0;
  virtual void addOperation(std::unique_ptr<Operation<InitIter>>&& op) = 0;
};

// TODO: Don't even need InitIter here because the Stream Class guarantees that it is a
// vector<Start>::iterator But an optimization would be to determine the final container based on
// what the collector is at the end.
template <typename InitIter, typename End, typename Prev>
class PartialStream final : public BasePartialStream<InitIter, End> {
public:
  using EndIter = typename std::vector<End>::iterator;

  template <typename Fn>
  PartialStream(std::unique_ptr<BasePartialStream<InitIter, Prev>>&& prevStream, Fn&& mapFn)
      : prevStream_(std::move(prevStream)), mapFn_(std::forward<Fn>(mapFn)) {}

  std::vector<End> process(InitIter* begin, InitIter* end) override;
  // vector<Prev> prevVec = prevStream.process(begin, end);
  // vector<End> vec = applyMapping(prevVec.begin(), prevVec.end());
  // apply operations_ to vec;

  virtual void addOperation(std::unique_ptr<Operation<EndIter>>&& op) {
    operations_.push_back(std::move(op));
  }

private:
  std::vector<End> applyMapping(InitIter begin, InitIter end) {
    std::vector<End> vec;
    std::transform(begin, end, std::back_inserter(vec), mapFn_);
    return vec;
  }

  std::unique_ptr<BasePartialStream<InitIter, Prev>> prevStream_;
  std::vector<std::unique_ptr<Operation<EndIter>>> operations_;
  // std::optional<std::function<End(const Prev&)>> mapFn_;
  std::function<End(const Prev&)> mapFn_;
};


template <typename T, typename InitIter>
class Stream final {
private:
  Stream(InitIter begin, InitIter end, std::unique_ptr<BasePartialStream<InitIter, T>>&& partial);

  InitIter begin_;
  InitIter end_;
  std::unique_ptr<BasePartialStream<InitIter, T>> partial_;

public:
  Stream(InitIter begin, InitIter end)
      : partial_(std::make_unique<PartialStream<InitIter, T, std::nullptr_t>>(
          nullptr,
          // Initial map operation makes a copy of the input container
          [](const T& obj) { return obj; })) {}

  template <typename Fn>
  Stream<T, InitIter>& filter(Fn&& filterFn) {
    partial_->addOperation(make_unique<FilterOp>(std::forward<Fn>(filterFn)));
  }

  template <typename Fn>
  auto map(Fn&& mapFn) {
    // TODO: Mark this stream as invalid
    using To = std::invoke_result_t<Fn, const T&>;
    return Stream<To, InitIter>(
        begin_,
        end_,
        std::make_unique<PartialStream<InitIter, T, To>>(
            std::move(partial_), std::forward<Fn>(mapFn)));
  }
};

#endif  // STREAM_HPP
