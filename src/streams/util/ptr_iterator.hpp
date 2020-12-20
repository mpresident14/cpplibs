#ifndef PTR_ITERATOR_HPP
#define PTR_ITERATOR_HPP

#include <vector>

template <typename T>
class PtrIterator {
  using BaseIter = typename std::vector<T>::iterator;
  using BasePtrIter = typename std::vector<T*>::iterator;

public:
  using iterator_category = typename BasePtrIter::iterator_category;
  using value_type = typename BaseIter::value_type;
  using difference_type = typename BasePtrIter::difference_type;
  using pointer = typename BaseIter::pointer;
  using reference = typename BaseIter::reference;
  PtrIterator(BasePtrIter iter) : iter_(iter) {}

  bool operator==(const PtrIterator& other) { return iter_ == other.iter_; }
  bool operator!=(const PtrIterator& other) { return iter_ != other.iter_; }

  PtrIterator& operator+=(const difference_type& diff) { return (iter_ += diff, *this); }
  PtrIterator& operator-=(const difference_type& diff) { return (iter_ -= diff, *this); }
  PtrIterator& operator++() { return (++iter_, *this); }
  PtrIterator& operator--() { return (--iter_, *this); }
  PtrIterator& operator++(int) {
    auto temp(*this);
    return (++iter_, temp);
  }
  PtrIterator& operator--(int) {
    auto temp(*this);
    return (--iter_, temp);
  }

  PtrIterator operator+(const difference_type& diff) { return PtrIterator(iter_ + diff); }
  PtrIterator operator-(const difference_type& diff) { return PtrIterator(iter_ - diff); }

  difference_type operator-(const PtrIterator& other) { return iter_ - other.iter_; }

  reference operator*() { return **iter_; }
  pointer operator->() { return *iter_; }


private:
  BasePtrIter iter_;
};

#endif  // PTR_ITERATOR_HPP
