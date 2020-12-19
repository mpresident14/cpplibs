#include "src/streams/stream.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;

int main() {
  using Container = std::array<int, 3>;
  Container container{ 1, 2, 3 };

  vector<size_t> result = prez::streams::streamFrom(container.begin(), container.end())
                              .map([](int n) { return "'" + to_string(n) + "'"; })
                              .map(std::mem_fn(&string::size))
                              .toVector();

  cout << result << endl;

  cout << "Ran successfully :)" << endl;

  return 0;
}
