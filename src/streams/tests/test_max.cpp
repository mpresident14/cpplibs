#include "src/streams/stream.hpp"
#include "src/testing/widget.hpp"
#include "src/testing/unit_test.hpp"

#include <array>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace prez::unit_test;

namespace ps = prez::streams;

array<int, 3> ARR = {28, 3, 5};

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

TEST(max_immediately) {
  int result = ps::streamFrom(ARR).max().value();

  assertEquals(28, result);
}

TEST(max_afterMap) {
  string result = ps::streamFrom(ARR).map(INT_TO_STRING).max().value();

  assertEquals("5", result);
}

TEST(max_empty) {
  vector<Widget> v;
  auto result = ps::streamFrom(v).max();

  assertFalse(result.has_value());
}

TEST(max_immediately_withComparator) {
  int result = ps::streamFrom(ARR).max(std::greater<>()).value();

  assertEquals(3, result);
}

TEST(max_afterMap_withComparator) {
  string result =
      ps::streamFrom(ARR)
          .map(INT_TO_STRING)
          .max([](const string& s1, const string& s2) { return s1.length() < s2.length(); })
          .value();

  assertEquals("28", result);
}

int main() { return runTests(); }
