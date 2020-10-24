#include "src/ctor_parser/ctor_parser.hpp"

#include <string>

#include <prez/unit_test.hpp>

using namespace std;

prez::UnitTest TESTER = prez::UnitTest::createTester();

void verifyResult(
    const ctor_parser::Result& result, const string& expectedClass, vector<string> expectedArgs) {
  TESTER.assertEquals(expectedClass, result.classType.name);
  TESTER.assertEquals(expectedArgs.size(), result.argTypes.size());
  for (size_t i = 0; i < expectedArgs.size(); ++i) {
    TESTER.assertEquals(expectedArgs[i], result.argTypes[i].name);
  }
}


void test_noParams() {
  ctor_parser::CtorParser parser;
  const char* ctor = R"(
    Foo () {}
  )";

  ctor_parser::Result result = parser.parseCtor(ctor, 0, "");

  verifyResult(result, "Foo", {});
}

void test_oneParam() {
  ctor_parser::CtorParser parser;
  const char* ctor = R"(
    Foo (Bar bar) {}
  )";


  ctor_parser::Result result = parser.parseCtor(ctor, 0, "");

  verifyResult(result, "Foo", {"Bar"});
}

void test_multipleParams() {
  ctor_parser::CtorParser parser;
  const char* ctor = R"(
    Foo (Bar bar, Baz c, CoolObj<int> cool_obj) {}
  )";


  ctor_parser::Result result = parser.parseCtor(ctor, 0, "");

  verifyResult(result, "Foo", {"Bar", "Baz", "CoolObj<int>"});
}


int main() {
  test_noParams();
  test_oneParam();
  test_multipleParams();
}
