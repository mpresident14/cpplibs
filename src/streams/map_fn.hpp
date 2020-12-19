#ifndef MAP_FN_HPP
#define MAP_FN_HPP

#include "src/misc/movable_fn.hpp"

#include <algorithm>
#include <type_traits>
#include <vector>

namespace prez {
namespace streams {
  namespace detail {

    template <typename From, typename To, typename InitIter>
    class MapFn {
    public:
      template <typename ElementMapper>
      static MapFn<From, To, InitIter> fromElemMapper(ElementMapper&& mapper) {
        return MapFn(std::forward<ElementMapper>(mapper), ELEMENT_MAPPER_TAG);
      };

      template <typename FullMapper>
      static MapFn<From, To, InitIter> fromFullMapper(FullMapper&& mapper) {
        return MapFn(std::forward<FullMapper>(mapper), FULL_MAPPER_TAG);
      }

      ~MapFn() = default;
      MapFn(const MapFn&) = delete;
      MapFn(MapFn&&) = default;
      MapFn& operator=(const MapFn&) = delete;
      MapFn& operator=(MapFn&&) = default;

      std::vector<To> apply(InitIter begin, InitIter end) { return mapFn_(begin, end); }

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
      static misc::MovableFn<std::vector<To>(InitIter, InitIter)> makeMapFn(
          ElementMapper&& mapper) {
        return [mapper = std::forward<ElementMapper>(mapper)](InitIter begin, InitIter end) {
          std::vector<To> outVec;
          std::transform(begin, end, std::back_inserter(outVec), mapper);
          return outVec;
        };
      }

      misc::MovableFn<std::vector<To>(InitIter, InitIter)> mapFn_;
    };

  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // MAP_FN_HPP
