#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "src/streams/operations/operation.hpp"

namespace prez {
namespace streams {
  namespace detail {

    template <typename T>
    using vecIter = typename std::vector<T>::iterator;

    template <typename T>
    class Operation {
    public:
      virtual ~Operation() = default;
      virtual void apply(vecIter<T>* begin, vecIter<T>* end) = 0;
    };


  }  // namespace detail
}  // namespace streams
}  // namespace prez
#endif  // OPERATION_HPP
