#ifndef PREZ_STREAMS_OPERATIONS_OPERATION_HPP
#define PREZ_STREAMS_OPERATIONS_OPERATION_HPP

#include "src/streams/operations/operation.hpp"

#include <type_traits>

namespace prez {
namespace streams {
namespace detail {

template <typename T>
using vecIter = typename std::vector<std::remove_cvref_t<T>>::iterator;

template <typename T> class Operation {
public:
  virtual ~Operation() = default;
  virtual void apply(vecIter<T>* begin, vecIter<T>* end) = 0;
};

} // namespace detail
} // namespace streams
} // namespace prez
#endif // OPERATION_HPP
