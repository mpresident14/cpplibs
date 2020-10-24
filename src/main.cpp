#include <iostream>

#include "src/ctor_parser/ctor_parser.hpp"

using namespace std;

int main() {
  ctor_parser::CtorParser parser;
  ctor_parser::Result result = parser.parseCtor("Foo(Bar b, Car c) { blah blah blah; }", __LINE__, __FILE__);
  cout << result.classType.name << endl;
  for (auto argType : result.argTypes) {
    cout << argType.name << endl;
  }
  cout << result.line << endl;
  cout << result.filename << endl;
}
