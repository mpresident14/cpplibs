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

array<int, 3> ARR = {28, 3, 534};

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

TEST(min_immediately) {
  int result = ps::streamFrom(ARR).min().value();

  assertEquals(3, result);
}

TEST(min_afterMap) {
  string result = ps::streamFrom(ARR).map(INT_TO_STRING).min().value();

  assertEquals("28", result);
}

TEST(min_empty) {
  vector<Widget> v;
  optional<reference_wrapper<Widget>> result = ps::streamFrom(v).min();

  assertFalse(result.has_value());
}

TEST(min_immediately_withComparator) {
  int result = ps::streamFrom(ARR).min(std::greater<>()).value();

  assertEquals(534, result);
}

TEST(min_afterMap_withComparator) {
  string result =
      ps::streamFrom(ARR)
          .map(INT_TO_STRING)
          .min([](const string& s1, const string& s2) { return s1.length() < s2.length(); })
          .value();

  assertEquals("3", result);
}

int main() { return runTests(); }
