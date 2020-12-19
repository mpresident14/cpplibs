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
using namespace unit_test;

namespace ps = prez::streams;


array<int, 10> ARR = { 28, 3, 5, 1, 1, 4, 4, 4, 5, 9 };

auto IS_EVEN = [](int n) { return n % 2 == 0; };
auto IS_DOUBLE_DIGIT = [](int n) { return n >= 10; };


TEST(filter_onceImmediately) {
  vector<int> expected = { 28, 4, 4, 4 };

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end()).filter(IS_EVEN).toVector();

  assertEquals(expected, result);
}

TEST(filter_twiceImmediately) {
  vector<int> expected = { 28 };

  vector<int> result =
      ps::streamFrom(ARR.begin(), ARR.end()).filter(IS_EVEN).filter(IS_DOUBLE_DIGIT).toVector();

  assertEquals(expected, result);
}

TEST(filter_nonCopyable) {
  vector<Widget> expected;
  for (int n : ARR) {
    expected.emplace_back(n);
  }

  vector<Widget> result =
      ps::streamFrom(ARR.begin(), ARR.end()).map([](int n) { return Widget(n); }).toVector();

  assertEquals(expected, result);
}

// TEST(filter_toFunction) {
//   int addend = 1410;
//   vector<int> expected;
//   for (int n : ARR) {
//     expected.push_back(n + addend);
//   }

//   vector<int> result = ps::streamFrom(ARR.begin(), ARR.end())
//                            .map([](int n) { return [n](int x) { return n + x; }; })
//                            .map([addend](const auto& adder) { return adder(addend); })
//                            .toVector();

//   assertEquals(expected, result);
// }


int main() { return runTests(); }
