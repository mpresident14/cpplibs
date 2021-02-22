#include "src/streams/stream.hpp"
#include "src/streams/testing/widget.hpp"
#include "src/testing/unit_test.hpp"

#include <array>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace prez::unit_test;

namespace ps = prez::streams;

array<int, 3> ARR = {28, 3, 5};

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

template <typename T>
concept CanCallSum = requires(std::vector<T> obj) {
  ps::streamFrom(obj.begin(), obj.end()).sum();
};


TEST(sum_immediately) {
  int result = ps::streamFrom(ARR).sum();

  assertEquals(36, result);
}

TEST(sum_afterMap) {
  string result = ps::streamFrom(ARR).map(INT_TO_STRING).sum();

  assertEquals("2835", result);
}

TEST(sum_nonAddable_doesNotCompile) {
  struct NoAdd {};

  assertFalse(CanCallSum<NoAdd>);
}

int main() { return runTests(); }
