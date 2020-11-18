#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <string>

using namespace std;
using namespace unit_test;

static int BASE = 14;
static int DERIVED = 999;

class Base {
public:
  int val = BASE;
  bool copied = false;

  INJECT(Base(int v1, int v2)) : val(v1 + v2) {}
  Base() = default;
  Base(const Base& other) : val(other.val), copied(true) {}
  Base(Base&& other) = default;
};

class Derived : public Base {
public:
  Derived() { val = DERIVED; }
  Derived(const Derived& other) : Base(other) {}
  Derived(Derived&& other) = default;
};


AFTER(clearBindings) { injector::clearBindings(); }


TEST(injectUnique_nonPtrProvider_throws) {
  injector::bindToProvider<Base>([]() { return Derived(); });

  assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
}

TEST(injectUnique_uniqueProvider_success) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectUnique_sharedProvider_throws) {
  injector::bindToProvider<Base>([]() { return make_shared<Derived>(); });

  assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
}

TEST(injectUnique_instance_throws) {
  injector::bindToInstance<Base>(make_shared<Derived>());

  string err = assertThrows([]() { injector::inject<unique_ptr<Base>>(); });
}

TEST(injectShared_nonPtrProvider_throws) {
  injector::bindToProvider<Derived>([]() { return Derived(); });

  assertThrows([]() { injector::inject<shared_ptr<Derived>>(); });
}

TEST(injectShared_uniqueProvider_success) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });

  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_sharedProvider_throws) {
  injector::bindToProvider<Base>([]() { return make_shared<Derived>(); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_instance_success) {
  injector::bindToInstance(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
  assertFalse(b->copied);
}

TEST(injectNonPtr_nonPtrProvider_success) {
  injector::bindToProvider<Base>([]() { return Derived(); });
  const Base& b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_uniqueProvider_success) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });
  Base b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_sharedProvider_success) {
  injector::bindToProvider<Base>([]() { return make_shared<Base>(); });
  const Base& b = injector::inject<Base>();

  assertEquals(BASE, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_instance_success) {
  injector::bindToInstance<Derived>(make_shared<Derived>());
  Derived d = injector::inject<Derived>();

  assertEquals(DERIVED, d.val);
  assertTrue(d.copied);
}

// TEST(injectUnique_byConstructor_success) {
//   int n = 234;

//   injector::bindToInstance(make_shared<int>(n));
//   unique_ptr<Base> b = injector::injectByConstructor<unique_ptr<Base>>();

//   assertEquals(n * 2, b->val);
// }

// TEST(injectShared_byConstructor_success) {
//   int n = 234;

//   injector::bindToInstance(make_shared<int>(n));
//   shared_ptr<Base> b = injector::injectByConstructor<shared_ptr<Base>>();

//   assertEquals(n * 2, b->val);
// }

// TEST(injectNonPtr_byConstructor_success) {
//   int n = 234;

//   injector::bindToInstance(make_shared<int>(n));
//   Base b = injector::injectByConstructor<Base>();

//   assertEquals(n * 2, b.val);
// }


TEST(inject_noBinding_throws) {
  string err = assertThrows([]() { injector::inject<shared_ptr<Base>>(); });
}

// TODO: Remove this after we add ctor injection to inject()
// TEST(hasInjectedCtor) {
//   assertTrue(injector::has_injected_ctor_v<Base>);
//   assertFalse(injector::has_injected_ctor_v<int>);
// }

int main() { runTests(); }
