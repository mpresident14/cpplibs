#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <string>

using namespace std;

prez::UnitTest TESTER = prez::UnitTest::createTester();

static int BASE = 0;
static int DERIVED = 1;

class Base {
public:
  int val = BASE;
};

class Derived : public Base {
public:
  Derived() { val = DERIVED; }
  // INJECT(Derived(int n, char c)) {}
  Derived(const Derived&) = delete;
  Derived(Derived&&) = delete;
};

void test_bindToInstance_sameType_success() {
  injector::clearBindings();

  injector::bindToInstance(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  TESTER.assertEquals(BASE, b->val);
}

void test_bindToInstance_derivedType_success() {
  injector::clearBindings();

  injector::bindToInstance<Base>(make_shared<Derived>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  TESTER.assertEquals(DERIVED, b->val);
}

void test_bindToInstance_uniquePtr_throws() {
  injector::clearBindings();

  injector::bindToInstance<Base>(make_shared<Derived>());

  string err = TESTER.assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
  TESTER.assertTrue(err.find("only a shared_ptr was bound") != string::npos);
}

void test_bindToUniqueProvider_sameType_success() {
  injector::clearBindings();

  injector::bindToProvider<int>([]() { return make_unique<int>(123456); });
  unique_ptr<int> n = injector::inject<unique_ptr<int>>();

  TESTER.assertEquals(*n, 123456);
}

void test_bindToUniqueProvider_derivedType_success() {
  injector::clearBindings();

  injector::bindToProvider<Base>([]() { return unique_ptr<Derived>(new Derived()); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  TESTER.assertEquals(DERIVED, b->val);
}

void test_bindToUniqueProvider_injectSharedPtr_success() {
  injector::clearBindings();

  injector::bindToProvider<Base>([]() { return make_unique<Base>(Derived()); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  TESTER.assertEquals(DERIVED, b->val);
}

void test_bindToSharedProvider_sameType_success() {
  injector::clearBindings();

  injector::bindToProvider<int>([]() { return make_shared<int>(123456); });
  shared_ptr<int> n = injector::inject<shared_ptr<int>>();

  TESTER.assertEquals(*n, 123456);
}

void test_bindToSharedProvider_derivedType_success() {
  injector::clearBindings();

  injector::bindToProvider<Base>([]() { return make_shared<Base>(Derived()); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  TESTER.assertEquals(DERIVED, b->val);
}

void test_bindToSharedProvider_injectUniquePtr_throws() {
  injector::clearBindings();

  injector::bindToProvider<Base>([]() { return make_shared<Base>(Derived()); });

  string err = TESTER.assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
  TESTER.assertTrue(err.find("only a shared_ptr was bound") != string::npos);
}

void test_inject_noBinding_throws() {
  injector::clearBindings();

  string err = TESTER.assertThrows([]() { injector::inject<shared_ptr<Base>>(); });
  TESTER.assertTrue(err.find("No binding") != string::npos);
}

int main() {
  test_bindToInstance_sameType_success();
  test_bindToInstance_derivedType_success();
  test_bindToInstance_uniquePtr_throws();
  test_bindToUniqueProvider_sameType_success();
  test_bindToUniqueProvider_derivedType_success();
  test_bindToUniqueProvider_injectSharedPtr_success();
  test_bindToSharedProvider_sameType_success();
  test_bindToSharedProvider_derivedType_success();
  test_bindToSharedProvider_injectUniquePtr_throws();
  test_inject_noBinding_throws();
}

// TODO: Rewrite unit_tester to take in void -> void functions and run them, also add @Before and
// @After functions (TESTER.setBefore(fn), TESTER.setAfter(fn), TESTER.run(fn1, fn2, ...))
