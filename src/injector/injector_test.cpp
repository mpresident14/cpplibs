#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <string>

using namespace std;
using namespace unit_test;

static int BASE = 0;
static int DERIVED = 1;

class Base {
public:
  int val = BASE;

  Base() = default;
  // Base(const Base&) = delete;
  // Base(Base&&) = default;
};

class Derived : public Base {
public:
  Derived() { val = DERIVED; }
  // INJECT(Derived(int n, char c)) {}
  Derived(const Derived&) = delete;
  Derived(Derived&&) = delete;
};

BEFORE(clearBindings) { injector::clearBindings(); }


TEST(bindToInstance_sameType_success) {
  injector::bindToInstance(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
}

TEST(bindToInstance_derivedType_success) {
  injector::bindToInstance<Base>(make_shared<Derived>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(bindToInstance_constRef_success) {
  injector::bindToInstance<Base>(make_shared<Derived>());
  const Base& b = injector::inject<const Base&>();

  assertEquals(DERIVED, b.val);
}

TEST(bindToInstance_ref_success) {
  injector::bindToInstance<Base>(make_shared<Derived>());
  Base& b = injector::inject<Base&>();

  assertEquals(DERIVED, b.val);
}

TEST(bindToInstance_uniquePtr_throws) {
  injector::bindToInstance<Base>(make_shared<Derived>());

  string err = assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
  assertTrue(err.find("only a shared_ptr was bound") != string::npos);
}

TEST(bindToUniqueProvider_sameType_success) {
  injector::bindToProvider<int>([]() { return make_unique<int>(123456); });
  unique_ptr<int> n = injector::inject<unique_ptr<int>>();

  assertEquals(*n, 123456);
}

TEST(bindToUniqueProvider_derivedType_success) {
  injector::bindToProvider<Base>([]() { return unique_ptr<Derived>(new Derived()); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(bindToUniqueProvider_injectSharedPtr_success) {
  injector::bindToProvider<Base>([]() { return make_unique<Base>(Derived()); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(bindToSharedProvider_sameType_success) {
  injector::bindToProvider<int>([]() { return make_shared<int>(123456); });
  shared_ptr<int> n = injector::inject<shared_ptr<int>>();

  assertEquals(*n, 123456);
}

TEST(bindToSharedProvider_derivedType_success) {
  injector::bindToProvider<Base>([]() { return make_shared<Base>(Derived()); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(bindToSharedProvider_injectUniquePtr_throws) {
  injector::bindToProvider<Base>([]() { return make_shared<Base>(Derived()); });

  string err = assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
  assertTrue(err.find("only a shared_ptr was bound") != string::npos);
}

TEST(bindToNonPtrProvider_sameType_success) {
  injector::bindToProvider<string>([]() { return "hello"; });

  string str = injector::inject<string>();
  assertEquals("hello", str);
}

TEST(bindToNonPtrProvider_derivedType_success) {
  injector::bindToProvider<Base>([]() { return Derived(); });

  Base b = injector::inject<Base>();
  assertEquals(DERIVED, b.val);
}

// TODO: Error tests

TEST(inject_noBinding_throws) {
  string err = assertThrows([]() { injector::inject<shared_ptr<Base>>(); });
  assertTrue(err.find("No binding") != string::npos);
}

int main() { runTests(); }
