#ifndef PREZ_MISC_PRINT_STUFF_HPP
#define PREZ_MISC_PRINT_STUFF_HPP

#include <array>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace prez {
namespace misc {
template <typename T> std::string arrayToString(T *arr, size_t length);
}
} // namespace prez

template <typename... Ts>
std::ostream &operator<<(std::ostream &out, const std::tuple<Ts...> &t);

template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &out, const std::pair<T1, T2> &p);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::deque<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::forward_list<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::list<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::set<T> &c);

template <typename... Ts>
std::ostream &operator<<(std::ostream &out, const std::unordered_set<Ts...> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::stack<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::queue<T> &c);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::priority_queue<T> &c);

template <typename T, int N>
std::ostream &operator<<(std::ostream &out, const std::array<T, N> &c);

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out, const std::unordered_map<K, V> &c);

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out, const std::map<K, V> &c);

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out,
                         const std::unordered_multimap<K, V> &c);

/********
 * IMPL *
 ********/

namespace prez {
namespace misc {
template <typename T> std::string arrayToString(T *arr, size_t length) {
  std::stringstream stream;
  stream << '[';

  for (size_t i = 0; i < length; ++i) {
    stream << arr[i];
    if (i != length - 1) {
      stream << ", ";
    }
  }
  stream << ']';
  return stream.str();
}
} // namespace misc
} // namespace prez

namespace detail {
template <typename Container>
std::ostream &printContainerHelper(std::ostream &out, const Container &c) {
  size_t length = c.size();
  size_t i = 0;
  out << '[';

  for (const auto &item : c) {
    out << item;
    if (i != length - 1) {
      out << ", ";
    }
    ++i;
  }
  return out << ']';
}

template <typename Map>
std::ostream &printMapHelper(std::ostream &out, const Map &c) {
  size_t length = c.size();
  size_t i = 0;
  out << "{\n";

  for (const auto &keyVal : c) {
    out << "\t(" << keyVal.first << ": " << keyVal.second << ')';
    if (i != length - 1) {
      out << ",\n";
    }
    ++i;
  }
  return out << "\n}";
}

template <size_t I, typename Tuple,
          std::enable_if_t<I == std::tuple_size_v<Tuple>, int> = 0>
void printTupleHelper(std::ostream &, const Tuple &) {}

template <size_t I, typename Tuple,
          std::enable_if_t<I == std::tuple_size_v<Tuple> - 1, int> = 0>
void printTupleHelper(std::ostream &out, const Tuple &t) {
  out << std::get<I>(t);
}

template <size_t I, typename Tuple,
          std::enable_if_t<(I < std::tuple_size_v<Tuple> - 1), int> = 0>
void printTupleHelper(std::ostream &out, const Tuple &t) {
  out << std::get<I>(t) << ", ";
  printTupleHelper<I + 1, Tuple>(out, t);
}
} // namespace detail

template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &out, const std::pair<T1, T2> &p) {
  return out << '(' << p.first << ", " << p.second << ')';
}

template <typename... Ts>
std::ostream &operator<<(std::ostream &out, const std::tuple<Ts...> &t) {
  out << '(';
  detail::printTupleHelper<0, std::tuple<Ts...>>(out, t);
  return out << ')';
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &c) {
  return detail::printContainerHelper(out, c);
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::deque<T> &c) {
  return detail::printContainerHelper(out, c);
}
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::forward_list<T> &c) {
  return detail::printContainerHelper(out, c);
}
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::list<T> &c) {
  return detail::printContainerHelper(out, c);
}
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::set<T> &c) {
  return detail::printContainerHelper(out, c);
}

// TODO: Do param pack for all containers
template <typename... Ts>
std::ostream &operator<<(std::ostream &out,
                         const std::unordered_set<Ts...> &c) {
  return detail::printContainerHelper(out, c);
}
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::stack<T> &c) {
  return detail::printContainerHelper(out, c);
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::queue<T> &c) {
  return detail::printContainerHelper(out, c);
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::priority_queue<T> &c) {
  return detail::printContainerHelper(out, c);
}

template <typename T, int N>
std::ostream &operator<<(std::ostream &out, const std::array<T, N> &c) {
  return detail::printContainerHelper(out, c);
}

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out, const std::unordered_map<K, V> &c) {
  return detail::printMapHelper(out, c);
}

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out, const std::map<K, V> &c) {
  return detail::printMapHelper(out, c);
}

template <typename K, typename V>
std::ostream &operator<<(std::ostream &out,
                         const std::unordered_multimap<K, V> &c) {
  return detail::printMapHelper(out, c);
}

#endif
