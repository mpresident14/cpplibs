#include "src/streams/stream.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace std;

int main() {
  auto mapFn1 = MapFn<int, int>::fromElemMapper([](int n) { return n; });
  auto sn1 = make_unique<StreamNode<int, int, std::tuple<>>>(
      nullptr, move(mapFn1), vector<unique_ptr<Operation<int>>>{});

  auto mapFn2 = MapFn<int, string>::fromElemMapper([](int n) { return to_string(n); });
  auto sn2 = make_unique<StreamNode<int, string, std::tuple<int>>>(
      move(sn1), move(mapFn2), vector<unique_ptr<Operation<string>>>{});


  StreamNode<int, string, std::tuple<>> combSn = sn2->combineAll();

  cout << "Ran successfully :)" << endl;

  return 0;
}
