#ifndef PREZ_MISC_OSTREAMABLE_HPP
#define PREZ_MISC_OSTREAMABLE_HPP

#include "src/misc/typing.hpp"

#include <ostream>
#include <string_view>
#include <tuple>
#include <utility>

namespace {


template <typename Obj>
concept MapIterable = requires(Obj obj) {
  obj.cbegin()->first;
  obj.cbegin()->second;
};

template <typename Obj>
concept NonMapIterable = requires(Obj obj) {
  obj.cbegin();
}
&&(!(MapIterable<Obj> || std::convertible_to<Obj, std::string_view>));

template <typename Obj>
concept Pair = requires(Obj obj) {
  obj.first;
  obj.second;
};

template <typename Obj>
concept Tuple = prez::misc::is_specialization_v<Obj, std::tuple>;

} // namespace

namespace prez {
namespace misc {

// TODO: This doesn't catch something like vector<NonprintableType>. Haven't found a good way to
// detect this yet.
template <typename U>
concept IsOStreamable = requires(std::ostream& out, U obj) {
  { out << obj }
  ->std::same_as<std::ostream&>;
}
|| MapIterable<U> || NonMapIterable<U> || Pair<U> || Tuple<U>;


template <typename T>
class OStreamable {
public:
  OStreamable(const T& thing) : thing_(thing) {}

  template <typename U>
  friend std::ostream& operator<<(std::ostream&, const OStreamable<U>&);

private:
  template <size_t I, typename Tuple, std::enable_if_t<I == std::tuple_size_v<Tuple>, int> = 0>
  static void printTupleHelper(std::ostream&, const Tuple&) {}

  template <size_t I, typename Tuple, std::enable_if_t<I == std::tuple_size_v<Tuple> - 1, int> = 0>
  static void printTupleHelper(std::ostream& out, const Tuple& t) {
    out << OStreamable<decltype(std::get<I>(t))>(std::get<I>(t));
  }

  template <size_t I, typename Tuple, std::enable_if_t<(std::tuple_size_v<Tuple> > 0 && I < std::tuple_size_v<Tuple> - 1), int> = 0>
  static void printTupleHelper(std::ostream& out, const Tuple& t) {
    out << OStreamable<decltype(std::get<I>(t))>(std::get<I>(t)) << ", ";
    printTupleHelper<I + 1, Tuple>(out, t);
  }

  template <typename U = T>
  requires MapIterable<U> void toStream(std::ostream& out) const {
    size_t length = thing_.size();
    size_t i = 0;
    out << "{\n";

    for (const auto& [key, val] : thing_) {
      out << "  (" << OStreamable<decltype(key)>(key) << ": " << OStreamable<decltype(val)>(val)
          << ')';
      if (i != length - 1) {
        out << ",\n";
      }
      ++i;
    }
    out << "\n}";
  }

  template <typename U = T>
  requires NonMapIterable<U> void toStream(std::ostream& out) const {
    size_t length = thing_.size();
    size_t i = 0;
    out << '[';

    for (const auto& item : thing_) {
      out << OStreamable<decltype(item)>(item);
      if (i != length - 1) {
        out << ", ";
      }
      ++i;
    }
    out << ']';
  }

  template <typename U = T>
  requires Pair<U> void toStream(std::ostream& out) const {
    out << '(' << OStreamable<decltype(thing_.first)>(thing_.first) << ", "
        << OStreamable<decltype(thing_.second)>(thing_.second) << ')';
  }

  template <typename... Args, typename U = T>
  requires Tuple<T> void toStream(std::ostream& out) const {
    out << '(';
    printTupleHelper<0>(out, thing_);
    out << ')';
  }

  template <typename U = T>
  requires(!(MapIterable<U> || NonMapIterable<U> || Pair<U> || Tuple<T>)) void toStream(
      std::ostream& out) const {
    out << thing_;
  }


  const T& thing_;
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const OStreamable<T>& ostreamable) {
  return (ostreamable.toStream(out), out);
}

} // namespace misc
} // namespace prez

#endif
