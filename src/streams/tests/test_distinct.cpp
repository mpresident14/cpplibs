#include "src/streams/stream.hpp"
#include "src/testing/unit_test.hpp"
#include "src/testing/widget.hpp"

#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace prez::unit_test;

namespace ps = prez::streams;

array<int, 10> ARR = {28, 3, 5, 1, 1, 4, 4, 4, 5, 9};

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

template <typename T>
concept CanCallDistinct = requires(std::vector<T> obj) {
  ps::streamFrom(obj.begin(), obj.end()).distinct();
};

class HashableThing {
public:
  HashableThing(int n) : num_(n) {}
  ~HashableThing() = default;
  HashableThing(const HashableThing&) = delete;
  HashableThing(HashableThing&&) = default;
  HashableThing& operator=(const HashableThing&) = delete;
  HashableThing& operator=(HashableThing&&) = default;

  int num_ = 0;
};

bool operator==(const HashableThing& w1, const HashableThing& w2) noexcept {
  return w1.num_ == w2.num_;
}

namespace std {
template <>
struct hash<HashableThing> {
  size_t operator()(const HashableThing& h) const { return hash<int>()(h.num_); }
};
} // namespace std

TEST(distinct_immediately) {
  vector<int> expected = {28, 3, 5, 1, 4, 9};

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end()).distinct().toVectorCopy();

  assertEquals(expected, result);
}

TEST(distinct_afterMap) {
  vector<string> expected = {"28", "3", "5", "1", "4", "9"};

  vector<string> result =
      ps::streamFrom(ARR.begin(), ARR.end()).map(INT_TO_STRING).distinct().toVector();

  assertEquals(expected, result);
}

TEST(distinct_comparableNonHashable) {
  vector<Widget> widgets;
  for (int n : ARR) {
    widgets.emplace_back(n);
  }
  vector<int> expected = {28, 3, 5, 1, 4, 9};

  vector<int> result = ps::streamFrom(widgets.begin(), widgets.end())
                           .distinct()
                           .map([](const Widget& w) { return w.num_; })
                           .toVector();

  assertEquals(expected, result);
}

TEST(distinct_hashableNonComparable) {
  vector<HashableThing> things;
  for (int n : ARR) {
    things.emplace_back(n);
  }
  vector<int> expected = {28, 3, 5, 1, 4, 9};

  vector<int> result = ps::streamFrom(things.begin(), things.end())
                           .distinct()
                           .map([](const HashableThing& h) { return h.num_; })
                           .toVector();

  assertEquals(expected, result);
}

TEST(distinct_customCompare) {
  vector<Widget> widgets;
  for (int n : ARR) {
    widgets.emplace_back(n);
  }
  auto compareNumberOfDigits = [](const Widget& w1, const Widget& w2) {
    return to_string(w1.num_).size() < to_string(w2.num_).size();
  };

  vector<int> expected = {28, 3};

  vector<int> result = ps::streamFrom(widgets.begin(), widgets.end())
                           .distinct(compareNumberOfDigits)
                           .map([](const Widget& w) { return w.num_; })
                           .toVector();

  assertEquals(expected, result);
}

TEST(distinct_customHash) {
  vector<Widget> widgets;
  for (int n : ARR) {
    widgets.emplace_back(n);
  }
  auto eqNumberOfDigits = [](const Widget& w1, const Widget& w2) {
    return to_string(w1.num_).size() == to_string(w2.num_).size();
  };
  auto hashNumberOfDigits = [](const Widget& w) { return to_string(w.num_).size(); };

  vector<int> expected = {28, 3};

  vector<int> result = ps::streamFrom(widgets.begin(), widgets.end())
                           .distinct(hashNumberOfDigits, eqNumberOfDigits)
                           .map([](const Widget& w) { return w.num_; })
                           .toVector();

  assertEquals(expected, result);
}

TEST(distinct_nonCopyableComp) {
  Widget w(1);
  vector<int> expected = {28, 3, 5, 1, 4, 9};
  auto comp = [w = std::move(w)](int n1, int n2) { return n1 + w.num_ < n2 + w.num_; };

  vector<int> result =
      ps::streamFrom(ARR.begin(), ARR.end()).distinct(std::move(comp)).toVectorCopy();

  assertEquals(expected, result);
}

TEST(distinct_nonCopyableHashAndEq) {
  Widget w(1);
  Widget x(2);
  vector<int> expected = {28, 3, 5, 1, 4, 9};
  auto eq = [w = std::move(w)](int n1, int n2) { return n1 + w.num_ == n2 + w.num_; };
  auto hash = [x = std::move(x)](int n) { return std::hash<int>{}(n + x.num_); };

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end())
                           .distinct(std::move(hash), std::move(eq))
                           .toVectorCopy();

  assertEquals(expected, result);
}

TEST(distinct_nonHashableNonComparable_doesNotCompile) {
  struct NoHashOrComp {};

  assertFalse(CanCallDistinct<NoHashOrComp>);
}

int main() { return runTests(); }
