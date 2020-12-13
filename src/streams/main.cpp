#include "src/streams/stream.hpp"

#include <iostream>

using namespace std;

int main() {
  using test = StreamNode<int, char, std::tuple<char, long>>;
  return 0;
}
