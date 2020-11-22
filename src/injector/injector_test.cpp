#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <string>


using namespace std;
using namespace unit_test;


class Unrelated final {
public:
  string str;
  INJECT(Unrelated(string s)) : str(s) {}
};

static int BASE = 14;
static int DERIVED = 999;

class Base {
public:
  int val = BASE;
  bool copied = false;

  Base() = default;
  Base(int v) : val(v) {}
  Base(const Base& other) : val(other.val), copied(true) {}
  Base(Base&& other) = default;
};

class Derived final : public Base {
public:
  Derived() : Base(DERIVED) {}
  INJECT(Derived(int v, Unrelated u)) : Base(v + u.str.size()) {}
  Derived(const Derived& other) : Base(other) {}
  Derived(Derived&& other) = default;
};


AFTER(tearDown) { injector::clearBindings(); }


TEST(injectUnique_nonPtrProvider) {
  Derived d;
  injector::bindToProvider<Base>([&d]() { return d; });

  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_uniqueProvider) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectUnique_sharedProvider) {
  injector::bindToProvider<Base>([]() { return make_shared<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_instance) {
  injector::bindToInstance<Base>(make_shared<Derived>());
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectShared_nonPtrProvider) {
  Derived d;
  injector::bindToProvider<Derived>([&d]() { return d; });

  unique_ptr<Base> b = injector::inject<unique_ptr<Derived>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectShared_uniqueProvider) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_sharedProvider) {
  injector::bindToProvider<Base>([]() { return make_shared<Derived>(); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_instance) {
  injector::bindToInstance(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
  assertFalse(b->copied);
}

TEST(injectNonPtr_nonPtrProvider) {
  injector::bindToProvider<Base>([]() { return Derived(); });
  const Base& b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_uniqueProvider) {
  injector::bindToProvider<Base>([]() { return make_unique<Derived>(); });
  Base b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_sharedProvider) {
  injector::bindToProvider<Base>([]() { return make_shared<Base>(); });
  const Base& b = injector::inject<Base>();

  assertEquals(BASE, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_instance) {
  injector::bindToInstance<Derived>(make_shared<Derived>());
  Derived d = injector::inject<Derived>();

  assertEquals(DERIVED, d.val);
  assertTrue(d.copied);
}

TEST(injectUnique_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToInstance(make_shared<int>(n));
  injector::bindToProvider<string>([&str]() { return str; });
  unique_ptr<Base> b = injector::inject<unique_ptr<Derived>>();

  assertEquals(n + str.size(), b->val);
}

TEST(injectShared_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToInstance(make_shared<int>(n));
  injector::bindToProvider<string>([&str]() { return str; });
  shared_ptr<Base> b = injector::inject<shared_ptr<Derived>>();

  assertEquals(n + str.size(), b->val);
}

TEST(injectNonPtr_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToInstance(make_shared<int>(n));
  injector::bindToProvider<string>([&str]() { return str; });
  Base b = injector::inject<Derived>();

  assertEquals(n + str.size(), b.val);
}

TEST(inject_noBinding_throws) {
  string err = assertThrows([]() { injector::inject<shared_ptr<Base>>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_wrongBinding_throws) {
  injector::bindToProvider<int>([]() { return make_shared<int>(5); });
  string err = assertThrows([]() { injector::inject<string>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(bind_multiple_throws) {
  injector::bindToProvider<int>([]() { return make_unique<int>(5); });
  string err = assertThrows([]() { injector::bindToInstance<int>(make_shared<int>(5)); });

  assertContains("already exists", err);
}

int main() { runTests(); }
