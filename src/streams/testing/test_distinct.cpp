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
vector<int> VEC(ARR.cbegin(), ARR.cend());
unordered_set<int> USET(ARR.cbegin(), ARR.cend());

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);
auto STRING_TO_INT = [](const string& str) { return std::stoi(str); };
auto IS_EVEN = [](int n) { return n % 2 == 0; };


TEST(map_onceImmediately) {
  vector<string> expected = { "28", "3", "5", "1", "1", "4", "4", "4", "5", "9" };

  vector<string> result = ps::streamFrom(ARR.begin(), ARR.end()).map(INT_TO_STRING).toVector();

  assertEquals(expected, result);
}

TEST(map_twiceImmediately) {
  vector<int> result =
      ps::streamFrom(VEC.begin(), VEC.end()).map(INT_TO_STRING).map(STRING_TO_INT).toVector();

  assertEquals(VEC, result);
}

TEST(map_onceAfterOp) {
  vector<string> expected = { "28", "4", "4", "4" };

  vector<string> result =
      ps::streamFrom(USET.begin(), USET.end()).filter(IS_EVEN).map(INT_TO_STRING).toVector();

  assertEquals(expected, result);
}

TEST(map_twiceAfterOp) {
  vector<int> expected = { 4, 4, 4 };

  vector<int> result = ps::streamFrom(ARR.begin(), ARR.end())
                           .filter(IS_EVEN)
                           .map(INT_TO_STRING)
                           .filter([](const string& str) { return str.size() == 1; })
                           .map(STRING_TO_INT)
                           .toVector();

  assertEquals(expected, result);
}

TEST(map_nonCopyable) {
  vector<Widget> expected;
  for (int n : USET) {
    expected.emplace_back(n);
  }

  vector<Widget> result =
      ps::streamFrom(USET.begin(), USET.end()).map([](int n) { return Widget(n); }).toVector();

  assertEquals(expected, result);
}


int main() { return runTests(); }
