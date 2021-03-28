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

auto IS_EVEN = [](int n) { return n % 2 == 0; };
auto IS_DOUBLE_DIGIT = [](int n) { return n >= 10; };
auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

TEST(filter_onceImmediately) {
  vector<int> expected = {28, 4, 4, 4};

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end()).filter(IS_EVEN).toVectorCopy();

  assertEquals(expected, result);
}

TEST(filter_twiceImmediately) {
  vector<int> expected = {28};

  vector<int> result =
      ps::streamFrom(ARR.begin(), ARR.end()).filter(IS_EVEN).filter(IS_DOUBLE_DIGIT).toVectorCopy();

  assertEquals(expected, result);
}

TEST(filter_afterMap) {
  vector<string> expected = {"28"};

  vector<string> result = ps::streamFrom(ARR.begin(), ARR.end())
                              .map(INT_TO_STRING)
                              .filter([](const string& str) { return str.size() > 1; })
                              .toVector();

  assertEquals(expected, result);
}

TEST(filter_nonCopyableFilter) {
  Widget w(4);
  vector<int> expected = {4, 4, 4};

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end())
                           .filter([w = std::move(w)](int n) { return n == w.num_; })
                           .toVectorCopy();

  assertEquals(expected, result);
}

int main() { return runTests(); }
