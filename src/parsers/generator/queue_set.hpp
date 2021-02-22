#ifndef PREZ_PARSERS_GENERATOR_QUEUE_SET_HPP
#define PREZ_PARSERS_GENERATOR_QUEUE_SET_HPP

#include <queue>
#include <unordered_set>

/* Quick and dirty implementation of a linked hashset */
template <typename T>
class QueueSet {
public:
  QueueSet() = default;
  ~QueueSet() = default;
  QueueSet(const QueueSet&) = default;
  QueueSet(QueueSet&&) = default;
  QueueSet& operator=(const QueueSet&) = default;
  QueueSet& operator=(QueueSet&&) = default;

  void push(const T& item);
  void push(T&& item);
  void pop();
  const T& front() const;

  bool empty() const noexcept;
  size_t size() const noexcept;

private:
  std::unordered_set<T> tSet_;
  std::queue<const T*> tQueue_;
};

template <typename T>
void QueueSet<T>::push(const T& item) {
  auto p = tSet_.insert(item);
  if (p.second) {
    tQueue_.push(&*p.first);
  }
}

template <typename T>
void QueueSet<T>::push(T&& item) {
  auto p = tSet_.insert(std::move(item));
  if (p.second) {
    tQueue_.push(&*p.first);
  }
}

template <typename T>
void QueueSet<T>::pop() {
  tSet_.erase(*tQueue_.front());
  tQueue_.pop();
}

template <typename T>
const T& QueueSet<T>::front() const {
  return *tQueue_.front();
}

template <typename T>
bool QueueSet<T>::empty() const noexcept {
  return tQueue_.empty();
}

template <typename T>
size_t QueueSet<T>::size() const noexcept {
  return tQueue_.size();
}

#endif // QUEUE_SET_HPP
