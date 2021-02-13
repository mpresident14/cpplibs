#include "src/streams/stream.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "src/misc/print_stuff.hpp"

using namespace std;

int main() {
  using Container = std::array<int, 10>;
  Container container{1, 4, 4, 4, 2, 1, 3, 4, 5, 6};

  vector<size_t> result =
      prez::streams::streamFrom(container.begin(), container.end())
          .distinct()
          .filter([](int n) { return n % 2 == 0; })
          .map([](int n) { return string(n, n + '0'); })
          .filter([](const string& str) { return str.size() > 3; })
          .map(std::mem_fn(&string::size))
          .toVector();

  cout << result << endl;

  cout << "Ran successfully :)" << endl;

  return 0;
}
