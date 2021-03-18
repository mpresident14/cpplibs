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
std::vector<int> VEC(ARR.cbegin(), ARR.cend());

auto INT_TO_STRING = static_cast<string (*)(int)>(std::to_string);

TEST(forEach_immediately) {
  std::vector<int> v;
  std::vector<int> result =
      ps::streamFrom(ARR).forEach([&v](int n) { v.push_back(n); }).toVectorCopy();

  assertEquals(VEC, v);
  assertEquals(VEC, result);
}

TEST(forEach_afterMap) {
  std::vector<string> result = ps::streamFrom(ARR)
                                   .map(INT_TO_STRING)
                                   .forEach([](string& str) { str.append("!"); })
                                   .toVectorCopy();

  assertEquals(std::vector<string>{"28!", "3!", "5!"}, result);
}

TEST(forEach_empty) {
  vector<string> v;

  vector<string> result = ps::streamFrom(v).forEach([](string&) {}).toVectorCopy();

  assertEquals(v, result);
}

TEST(filter_nonCopyableConsumer) {
  Widget w(1);
  vector<int> v(ARR.cbegin(), ARR.cend());
  vector<int> expected = {29, 4, 6};

  vector<int> result = ps::streamFrom(v.begin(), v.end())
                           .forEach([w = std::move(w)](int& n) { n += w.num_; })
                           .toVectorCopy();

  assertEquals(expected, result);
  assertEquals(v, result);
}


int main() { return runTests(); }
