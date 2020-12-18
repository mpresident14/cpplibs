#include "src/streams/stream.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;

int main() {
  using Container = std::array<int, 3>;
  using Iter = typename Container::iterator;
  Container v{ 1, 2, 3 };
  Iter startIter = v.begin();
  Iter endIter = v.end();

  auto sn1 = streamFrom(startIter, endIter);

  auto mapFn1 = MapFn<int, string, std::vector<int>::iterator>::fromElemMapper(
      [](int n) { return "'" + to_string(n) + "'"; });
  auto sn2 = make_unique<StreamNode<int, string, std::tuple<int>, Iter>>(
      startIter, endIter, move(sn1), move(mapFn1), vector<unique_ptr<Operation<string>>>{});

  auto mapFn2 = MapFn<string, long, std::vector<string>::iterator>::fromElemMapper(
      [](const string& str) { return str.size(); });
  auto sn3 = make_unique<StreamNode<string, long, std::tuple<int, int>, Iter>>(
      startIter, endIter, move(sn2), move(mapFn2), vector<unique_ptr<Operation<long>>>{});


  StreamNode<int, long, std::tuple<>, Iter> combSn = sn3->combineAll();
  vector<long> result = combSn.toVector();

  cout << result << endl;

  cout << "Ran successfully :)" << endl;

  return 0;
}

// #include <functional>
// #include <iostream>
// #include <string>
// using namespace std;
// int main() {
//   using Iter = vector<int>::iterator;

//   std::function<string(Iter, Iter)> fn = [](Iter a, Iter b) { return "hey"; };

//   vector<int> v;
//   cout << fn(v.begin(), v.end()) << endl;

//   return 0;
// }
