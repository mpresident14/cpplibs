#include "src/injector/injector.hpp"

#include <iostream>

using namespace std;


class Foo {
public:
  Foo() = default;
  INJECT(Foo(int n, char c)) {}
};

int main() {
  Foo f;
  return 0;
}
