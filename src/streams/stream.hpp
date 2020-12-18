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
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// template <typename Iter>
// class Operation {
// public:
//   virtual ~Operation() = default;
//   virtual void apply(Iter* begin, Iter* end) = 0;
// };

// template <typename T, template <typename> class Iter>
// class FilterOp : public Operation<Iter<T>> {
// public:
//   template <typename Fn>
//   FilterOp(Fn&& filterFn) : filterFn_(std::forward<Fn>(filterFn)) {}

//   virtual void apply(Iter<T>* begin, Iter<T>* end) = 0;

// private:
// std::function<void(const T&)> filterFn_;
// };

/*************
 * MovableFn *
 *************/

template <typename R, typename... Args>
class FnWrapper {
public:
  virtual ~FnWrapper() {}
  virtual R operator()(Args... args) = 0;
};

template <typename Fn, typename R, typename... Args>
class FnWrapperImpl : public FnWrapper<R, Args...> {
public:
  FnWrapperImpl(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}

  FnWrapperImpl(const FnWrapperImpl&) = delete;
  FnWrapperImpl(FnWrapperImpl&&) = delete;
  FnWrapperImpl& operator=(const FnWrapperImpl&) = delete;
  FnWrapperImpl& operator=(FnWrapperImpl&&) = delete;

  R operator()(Args... args) override { return fn_(args...); }

private:
  Fn fn_;
};

template <typename R, typename... Args>
class MovableFn;

template <typename R, typename... Args>
class MovableFn<R(Args...)> {
public:
  template <typename Fn>
  static MovableFn<R(Args...)> create(Fn&& fn) {
    return MovableFn(std::forward<Fn>(fn), CTOR_TAG);
  }


  MovableFn(const MovableFn&) = delete;
  MovableFn(MovableFn&&) = default;
  MovableFn& operator=(const MovableFn&) = delete;
  MovableFn& operator=(MovableFn&&) = default;

  R operator()(Args... args) { return (*fnWrapper_)(args...); }

private:
  struct CtorTag {};
  static constexpr CtorTag CTOR_TAG{};

  template <
      typename Fn,
      std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Fn, Args...>, R>, int> = 0>
  MovableFn(Fn&& fn, CtorTag)
      : fnWrapper_(std::make_unique<FnWrapperImpl<Fn, R, Args...>>(std::forward<Fn>(fn))) {}

  std::unique_ptr<FnWrapper<R, Args...>> fnWrapper_;
};


template <typename T>
using vecIter = typename std::vector<T>::iterator;

template <typename T>
class Operation {
public:
  virtual ~Operation() = default;
  virtual void apply(vecIter<T>* begin, vecIter<T>* end) = 0;
};

template <typename From, typename To, typename Iter>
class MapFn {
public:
  template <typename ElementMapper>
  static MapFn<From, To, Iter> fromElemMapper(ElementMapper&& mapper) {
    return MapFn(std::forward<ElementMapper>(mapper), ELEMENT_MAPPER_TAG);
  };

  template <
      typename FullMapper,
      typename = std::enable_if_t<std::is_rvalue_reference_v<FullMapper&&>>>
  static MapFn<From, To, Iter> fromFullMapper(FullMapper&& mapper) {
    return MapFn(std::forward<FullMapper>(mapper), FULL_MAPPER_TAG);
  }

  ~MapFn() = default;
  MapFn(MapFn&&) = default;

  std::vector<To> apply(Iter begin, Iter end) { return mapFn_(begin, end); }

private:
  struct ElementMapperTag {};
  struct FullMapperTag {};
  static constexpr FullMapperTag FULL_MAPPER_TAG{};
  static constexpr ElementMapperTag ELEMENT_MAPPER_TAG{};

  template <typename ElementMapper>
  MapFn(ElementMapper&& mapper, ElementMapperTag)
      : mapFn_(makeMapFn(std::forward<ElementMapper>(mapper))) {}

  template <typename FullMapper>
  MapFn(FullMapper&& mapper, FullMapperTag)
      : mapFn_(MovableFn<std::vector<To>(Iter, Iter)>::create(std::forward<FullMapper>(mapper))) {}

  template <typename ElementMapper>
  static MovableFn<std::vector<To>(Iter, Iter)> makeMapFn(ElementMapper&& mapper) {
    return MovableFn<std::vector<To>(Iter, Iter)>::create(
        [mapper = std::forward<ElementMapper>(mapper)](Iter begin, Iter end) {
          std::vector<To> outVec;
          std::transform(begin, end, std::back_inserter(outVec), mapper);
          return outVec;
        });
  }

  MovableFn<std::vector<To>(Iter, Iter)> mapFn_;
};

/***************
 * Type Traits *
 ***************/

template <typename Tuple, typename IfEmpty, typename Else>
using cond_tuple_empty_t = std::conditional_t<std::tuple_size_v<Tuple> == 0, IfEmpty, Else>;

template <typename PrevTuple>
struct first_arg {
  using type = std::tuple_element_t<0, PrevTuple>;
};

template <>
struct first_arg<std::tuple<>> {
  using type = std::nullptr_t;
};

template <typename PrevTuple>
using first_arg_t = typename first_arg<PrevTuple>::type;

template <typename PrevTuple>
struct last_arg {
  using type = std::tuple_element_t<std::tuple_size_v<PrevTuple> - 1, PrevTuple>;
};

template <>
struct last_arg<std::tuple<>> {
  using type = std::nullptr_t;
};

template <typename PrevTuple>
using last_arg_t = typename last_arg<PrevTuple>::type;

template <typename Tuple, typename IndexSeq>
struct subtuple;

template <typename Tuple, size_t... Is>
struct subtuple<Tuple, std::index_sequence<Is...>> {
  using type = std::tuple<std::tuple_element_t<Is, Tuple>...>;
};

template <size_t... Is>
struct subtuple<std::tuple<>, std::index_sequence<Is...>> {
  using type = std::tuple<>;
};

template <typename Tuple, size_t Trim>
using subtuple_t = typename subtuple<
    Tuple,
    std::make_index_sequence<std::tuple_size_v<Tuple> == 0 ? 0 : std::tuple_size_v<Tuple> - Trim>>::
    type;

template <typename Tuple>
using subtuple1_t = subtuple_t<Tuple, 1>;


/**************
 * StreamNode *
 **************/
template <typename From, typename To, typename PrevTuple, typename Iter>
class StreamNode {
private:
  using PrevSNPtr =
      std::unique_ptr<StreamNode<last_arg_t<PrevTuple>, From, subtuple1_t<PrevTuple>, Iter>>;
  // Initial node will use user-supplied iterator. Subsequent nodes will use the iterator of the
  // internal container (a vector).
  using ThisMapFn = MapFn<From, To, cond_tuple_empty_t<PrevTuple, Iter, vecIter<From>>>;
  template <typename From2, typename To2, typename PrevTuple2, typename Iter2>
  friend class StreamNode;

public:
  StreamNode(
      Iter begin,
      Iter end,
      PrevSNPtr&& prev,
      ThisMapFn&& mapFn,
      std::vector<std::unique_ptr<Operation<To>>>&& ops)
      : begin_(begin),
        end_(end),
        prev_(std::move(prev)),
        mapFn_(std::move(mapFn)),
        ops_(std::move(ops)) {}
  StreamNode(const StreamNode&) = delete;
  StreamNode(StreamNode&&) = default;
  StreamNode& operator=(const StreamNode&) = delete;
  StreamNode& operator=(StreamNode&&) = default;

  std::vector<To> toVector() {
    std::vector<To> toVec = mapFn_.apply(begin_, end_);
    vecIter<To> startIter = toVec.begin();
    vecIter<To> endIter = toVec.end();
    for (const auto& op : ops_) {
      op->apply(&startIter, &endIter);
    }
    return std::vector<To>(startIter, endIter);
  }


  // SFINAE needs to be applied on a template
  template <typename PrevTuple2 = PrevTuple>
  std::enable_if_t<
      std::tuple_size_v<PrevTuple2> != 0,
      StreamNode<first_arg_t<PrevTuple2>, To, std::tuple<>, Iter>>
  combineAll() {
    using PrevFrom = first_arg_t<PrevTuple>;

    // Cannot call on the initial StreamNode (should not reach this due to SFINAE)
    if (!prev_) {
      throw std::runtime_error("StreamNode::combineAll: nullptr prev_");
    }

    using NewPrevTuple = subtuple1_t<PrevTuple>;
    StreamNode<last_arg_t<PrevTuple>, To, subtuple1_t<PrevTuple>, Iter> combinedNode(
        begin_,
        end_,
        std::move(prev_->prev_),
        MapFn<PrevFrom, To, cond_tuple_empty_t<NewPrevTuple, Iter, vecIter<PrevFrom>>>::
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

    if constexpr (std::tuple_size_v<PrevTuple> == 1) {
      return combinedNode;
    }
    return combinedNode.combineAll();
  }

  template <typename PrevTuple2 = PrevTuple>
  std::enable_if_t<std::tuple_size_v<PrevTuple2> == 0, StreamNode<From, To, std::tuple<>, Iter>>
  combineAll() {
    throw std::runtime_error("StreamNode::combineAll: empty PrevTuple");
  }

private:
  Iter begin_, end_;
  PrevSNPtr prev_;
  ThisMapFn mapFn_;
  std::vector<std::unique_ptr<Operation<To>>> ops_;
};

template <typename Iter, typename T = std::remove_reference_t<decltype(*std::declval<Iter>())>>
static std::unique_ptr<StreamNode<T, T, std::tuple<>, Iter>> streamFrom(Iter begin, Iter end) {
  return make_unique<StreamNode<T, T, std::tuple<>, Iter>>(
      begin,
      end,
      nullptr,
      MapFn<T, T, Iter>::fromElemMapper([](const T& obj) { return obj; }),
      std::vector<std::unique_ptr<Operation<T>>>());
}

#endif  // STREAM_HPP
