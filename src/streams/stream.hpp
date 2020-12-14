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
//   std::function<void(const T&)> filterFn_;
// };

template <typename T>
using vecIter = typename std::vector<T>::iterator;

template <typename T>
class Operation {
public:
  virtual ~Operation() = default;
  virtual void apply(vecIter<T>* begin, vecIter<T>* end) = 0;
};


template <typename From, typename To>
class MapFn {
public:
  template <typename ElementMapper>
  static MapFn<From, To> fromElemMapper(ElementMapper&& mapper) {
    return MapFn(std::forward<ElementMapper>(mapper), ELEMENT_MAPPER_TAG);
  };

  template <typename FullMapper>
  static MapFn<From, To> fromFullMapper(FullMapper&& mapper) {
    return MapFn(std::forward<FullMapper>(mapper), FULL_MAPPER_TAG);
  }

  ~MapFn() = default;
  MapFn(MapFn&&) = default;

  std::vector<To> apply(vecIter<From>* begin, vecIter<From>* end) { return mapFn_(begin, end); }

private:
  struct ElementMapperTag {};
  struct FullMapperTag {};
  static constexpr FullMapperTag FULL_MAPPER_TAG{};
  static constexpr ElementMapperTag ELEMENT_MAPPER_TAG{};

  template <typename ElementMapper>
  MapFn(ElementMapper&& mapper, ElementMapperTag) : mapFn_(makeMapFn(mapper)) {}

  template <typename FullMapper>
  MapFn(FullMapper&& mapper, FullMapperTag) : mapFn_(mapper) {}

  template <typename ElementMapper>
  static std::function<std::vector<To>(vecIter<From>*, vecIter<From>*)> makeMapFn(
      ElementMapper&& mapper) {
    return
        [mapper = std::forward<ElementMapper>(mapper)](vecIter<From>* begin, vecIter<From>* end) {
          std::vector<To> outVec;
          std::transform(*begin, *end, std::back_inserter(outVec), mapper);
          return outVec;
        };
  }

  std::function<std::vector<To>(vecIter<From>* begin, vecIter<From>* end)> mapFn_;
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

template <typename From, typename To, typename PrevTuple>
class StreamNode;

template <typename From, typename To, typename PrevTuple>
using SNPtr = std::unique_ptr<StreamNode<From, To, PrevTuple>>;

/**************
 * StreamNode *
 **************/
template <typename From, typename To, typename PrevTuple>
class StreamNode {
  using PrevSNPtr = SNPtr<last_arg_t<PrevTuple>, From, subtuple1_t<PrevTuple>>;
  template <typename From2, typename To2, typename PrevTuple2>
  friend class StreamNode;

public:
  // TODO: MapFn should be ptr ???
  // NOTE TO SELF: Initial node will have a map fn that converts iterators to a vector (need to make
  // a defensive copy anyway in most cases)
  StreamNode(
      PrevSNPtr&& prev, MapFn<From, To>&& mapFn, std::vector<std::unique_ptr<Operation<To>>>&& ops)
      : prev_(std::move(prev)), mapFn_(std::move(mapFn)), ops_(std::move(ops)) {}
  StreamNode(const StreamNode&) = delete;
  StreamNode(StreamNode&&) = default;
  StreamNode& operator=(const StreamNode&) = delete;
  StreamNode& operator=(StreamNode&&) = default;


  // SFINAE needs to be applied on a template
  template <typename PrevTuple2 = PrevTuple>
  std::enable_if_t<
      std::tuple_size_v<PrevTuple2> != 0,
      StreamNode<first_arg_t<PrevTuple2>, To, std::tuple<>>>
  combineAll() {
    using PrevFrom = first_arg_t<PrevTuple>;

    // Cannot call on the initial StreamNode (should not reach this due to SFINAE)
    if (!prev_) {
      throw std::runtime_error("StreamNode::combineAll: nullptr prev_");
    }

    auto newMapFn = [this](vecIter<PrevFrom>* begin, vecIter<PrevFrom>* end) {
      std::vector<From> outVec = prev_->mapFn_.apply(begin, end);
      vecIter<From> startIter = outVec.begin();
      vecIter<From> endIter = outVec.end();
      for (const auto& op : prev_->ops_) {
        op->apply(&startIter, &endIter);
      }
      return mapFn_.apply(&startIter, &endIter);
    };

    StreamNode<last_arg_t<PrevTuple>, To, subtuple1_t<PrevTuple>> combinedNode(
        std::move(prev_->prev_),
        MapFn<PrevFrom, To>::fromFullMapper(std::move(newMapFn)),
        std::move(ops_));
    if constexpr (std::tuple_size_v<PrevTuple> == 1) {
      return combinedNode;
    }
    return combinedNode.combineAll();
  }

  template <typename PrevTuple2 = PrevTuple>
  std::enable_if_t<std::tuple_size_v<PrevTuple2> == 0, StreamNode<From, To, std::tuple<>>>
  combineAll() {
    throw std::runtime_error("StreamNode::combineAll: empty PrevTuple");
  }

private:
  PrevSNPtr prev_;
  MapFn<From, To> mapFn_;
  std::vector<std::unique_ptr<Operation<To>>> ops_;
};


#endif  // STREAM_HPP
