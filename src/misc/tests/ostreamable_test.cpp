#include "src/misc/ostreamable.hpp"
#include "src/testing/unit_test.hpp"

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;
using namespace prez::unit_test;
using namespace prez::misc;

TEST(ostream_default) {
  int thing = 54;
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "54");
}

TEST(ostream_nonMap) {
  vector<int> thing = {28, 3, 5, 1, 4, 9};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "[28, 3, 5, 1, 4, 9]");
}

TEST(ostream_nonMap_nested) {
  vector<vector<int>> thing = {{28}, {3, 5}, {1, 4, 9}};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "[[28], [3, 5], [1, 4, 9]]");
}

TEST(ostream_map) {
  unordered_map<int, string> thing = {{24, "hello"}, {84, "goodbye"}};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "{\n  (84: goodbye),\n  (24: hello)\n}");
}

TEST(ostream_map_nested) {
  unordered_map<int, vector<string>> thing = {{24, {"hello"}}, {84, {"goodbye", "see-ya"}}};
  stringstream out;

  out << OStreamable(thing);
}

TEST(ostream_pair) {
  pair<string, int> thing = {"hello", 365};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "(hello, 365)");
}

TEST(ostream_pair_nested) {
  pair<vector<string>, unordered_map<int, int>> thing = {{}, {{654, 234}, {1, 2}}};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "([], {\n  (1: 2),\n  (654: 234)\n})");
}

TEST(ostream_tuple) {
  tuple<int, string, int> thing = {1, "hello", 365};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "(1, hello, 365)");
}

TEST(ostream_tuple_nested) {
  tuple<int, vector<string>, tuple<int, int>> thing = {1, {}, {654, 234}};
  stringstream out;

  out << OStreamable(thing);

  assertEquals(out.str(), "(1, [], (654, 234))");
}

TEST(isOStreamable) {
  struct NotStreamableType {};

  assertFalse(IsOStreamable<NotStreamableType>);
}

int main() { return runTests(); }
