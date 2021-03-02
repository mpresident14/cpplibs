#ifndef PREZ_STREAMS_MAP_FN_HPP
#define PREZ_STREAMS_MAP_FN_HPP

#include "src/misc/movable_fn.hpp"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace prez {
namespace streams {
namespace detail {

template <typename To, typename Iter>
class MapFn {
public:
  template <typename ElementMapper>
  static MapFn<To, Iter> fromElemMapper(ElementMapper&& mapper) {
    return MapFn(std::forward<ElementMapper>(mapper), ELEMENT_MAPPER_TAG);
  };

  template <typename FullMapper>
  static MapFn<To, Iter> fromFullMapper(FullMapper&& mapper) {
    return MapFn(std::forward<FullMapper>(mapper), FULL_MAPPER_TAG);
  }

  ~MapFn() = default;
  MapFn(const MapFn&) = delete;
  MapFn(MapFn&&) = default;
  MapFn& operator=(const MapFn&) = delete;
  MapFn& operator=(MapFn&&) = default;

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
  MapFn(FullMapper&& mapper, FullMapperTag) : mapFn_(std::forward<FullMapper>(mapper)) {}

  template <typename ElementMapper>
  static misc::MovableFn<std::vector<To>(Iter, Iter)> makeMapFn(ElementMapper&& mapper) {
    return [mapper = std::forward<ElementMapper>(mapper)](Iter begin, Iter end) mutable {
      std::vector<To> outVec;
      std::transform(begin, end, std::back_inserter(outVec), std::ref(mapper));
      return outVec;
    };
  }

  misc::MovableFn<std::vector<To>(Iter, Iter)> mapFn_;
};

} // namespace detail
} // namespace streams
} // namespace prez

#endif // MAP_FN_HPP
