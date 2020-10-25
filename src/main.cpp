#include "src/injector/injector.hpp"

#include <iostream>

using namespace std;


class Foo {
public:
  Foo() = default;
  INJECT(Foo(int n, char c)) {}
  Foo(const Foo&) = delete;
  Foo(Foo&&) = delete;

  int a = 123456;
};

int main() {
  int n = 1;
  injector::bindToInstance(n);
  injector::bindToInstancePtr(make_unique<string>("hello"));
  int m = injector::inject<int>();
  string& str = injector::inject<string>();
  cout << m << endl;
  cout << str << endl;
  // Foo f;
  injector::bindToInstancePtr(unique_ptr<Foo>(new Foo()));
  Foo& foo = injector::inject<Foo>();
  cout << foo.a << endl;
  return 0;
}
