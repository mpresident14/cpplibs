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


TEST(reduce_immediately) {
  int result = ps::streamFrom(ARR).reduce(0, std::plus<>());

  assertEquals(36, result);
}

TEST(reduce_afterMap) {
  string result = ps::streamFrom(ARR)
                      .map(INT_TO_STRING)
                      .reduce<string>("Nums", [](string_view s1, string_view s2) {
                        return string(s1).append("_").append(s2);
                      });

  assertEquals("Nums_28_3_5", result);
}

TEST(reduce_empty) {
  vector<int> v;
  int result = ps::streamFrom(v).reduce(1, std::plus<>());

  assertEquals(1, result);
}

int main() { return runTests(); }
