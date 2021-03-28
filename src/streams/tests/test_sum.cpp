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

array<int, 3> ARR = {28, 3, 5};

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);


TEST(sum_immediately) {
  int result = ps::streamFrom(ARR).sum();

  assertEquals(36, result);
}

TEST(sum_afterMap) {
  string result = ps::streamFrom(ARR).map(INT_TO_STRING).sum();

  assertEquals("2835", result);
}

TEST(sum_empty) {
  vector<int> v;
  int result = ps::streamFrom(v).sum();

  assertEquals(0, result);
}

int main() { return runTests(); }
