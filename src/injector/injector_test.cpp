#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <sstream>
#include <string>


using namespace std;
using namespace unit_test;


class Unrelated final {
public:
  string str;
  ANNOTATED(int)
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

template <typename T, typename... Types>
string errorChain();

AFTER(tearDown) { injector::clearBindings(); }


TEST(injectUnique_nonPtrSupplier) {
  Derived d;
  injector::bindToSupplier<Base>([&d]() { return d; });

  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectUnique_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_sharedObject) {
  injector::bindToObject<Base>(make_shared<Derived>());
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_object) {
  injector::bindToObject<Base>(Derived());
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectUnique_impl) {
  int n = 234;
  string str = "ninechars";

  injector::bindToObject(n);
  injector::bindToSupplier<string>([&str]() { return str; });
  injector::bindToClass<Base, Derived>();
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(n + str.size(), b->val);
  assertFalse(b->copied);
}

TEST(injectUnique_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToObject(n);
  injector::bindToSupplier<string>([&str]() { return str; });
  unique_ptr<Base> b = injector::inject<unique_ptr<Derived>>();

  assertEquals(n + str.size(), b->val);
}

TEST(injectShared_nonPtrSupplier) {
  Derived d;
  injector::bindToSupplier<Derived>([&d]() { return d; });

  unique_ptr<Base> b = injector::inject<unique_ptr<Derived>>();

  assertEquals(DERIVED, b->val);
  assertTrue(b->copied);
}

TEST(injectShared_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_sharedObject) {
  injector::bindToObject(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
  assertFalse(b->copied);
}

TEST(injectShared_object) {
  injector::bindToObject(Base());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
  assertTrue(b->copied);
}

TEST(injectShared_impl) {
  injector::bindToSupplier<Derived>([]() { return make_shared<Derived>(); });
  injector::bindToClass<Base, Derived>();
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
  assertFalse(b->copied);
}


TEST(injectShared_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToObject(n);
  injector::bindToSupplier<string>([&str]() { return str; });
  shared_ptr<Base> b = injector::inject<shared_ptr<Derived>>();

  assertEquals(n + str.size(), b->val);
}

TEST(injectNonPtr_nonPtrSupplier) {
  injector::bindToSupplier<Base>([]() { return Derived(); });
  const Base& b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertFalse(b.copied);
}

TEST(injectNonPtr_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  Base b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
  assertTrue(b.copied);
}

TEST(injectNonPtr_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Base>(); });
  const Base& b = injector::inject<Base>();

  assertEquals(BASE, b.val);
  assertTrue(b.copied);
}

TEST(injectNonPtr_sharedObject) {
  auto ptr = make_shared<Derived>();

  injector::bindToObject<Derived>(ptr);
  Derived d = injector::inject<Derived>();

  assertEquals(DERIVED, d.val);
  assertTrue(d.copied);
}

TEST(injectNonPtr_Object) {
  injector::bindToObject(Derived());
  Derived d = injector::inject<Derived>();

  assertEquals(DERIVED, d.val);
  assertTrue(d.copied);
}

TEST(injectNonPtr_impl) {
  int n = 234;
  string str = "ninechars";

  injector::bindToObject(n);
  injector::bindToSupplier<string>([&str]() { return str; });
  injector::bindToClass<Base, Derived>();
  Base b = injector::inject<Base>();

  assertEquals(n + str.size(), b.val);
  assertFalse(b.copied);
}


TEST(injectNonPtr_byConstructor) {
  int n = 234;
  string str = "ninechars";

  injector::bindToObject(n);
  injector::bindToSupplier<string>([&str]() { return str; });
  Base b = injector::inject<Derived>();

  assertEquals(n + str.size(), b.val);
}

TEST(inject_noBinding_throws) {
  string err = assertThrows([]() { injector::inject<shared_ptr<Base>>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_wrongBinding_throws) {
  injector::bindToSupplier<int>([]() { return make_shared<int>(5); });
  string err = assertThrows([]() { injector::inject<string>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_noBindingNested_throwsWithCorrectInjectionChain) {
  injector::bindToSupplier<int>([]() { return 3; });
  string err = assertThrows([]() { injector::inject<Derived>(); });

  assertContains(errorChain<Derived, Unrelated, string>(), err);
}

TEST(inject_noBinding_classBinding_throwsWithCorrectInjectionChain) {
  injector::bindToSupplier<string>([]() { return ""; });
  injector::bindToClass<Base, Derived>();
  string err = assertThrows([]() { injector::inject<Base>(); });

  assertContains(errorChain<Base, Derived, int>(), err);
}

TEST(bind_multiple_throws) {
  injector::bindToSupplier<int>([]() { return make_unique<int>(5); });
  string err = assertThrows([]() { injector::bindToObject<int>(make_shared<int>(5)); });

  assertContains("already exists", err);
}

template <typename T, typename... Types>
string errorChain() {
  std::ostringstream out;
  out << "Injection chain:\n\t" << typeid(T).name();
  (..., (out << " -> " << typeid(Types).name()));
  out << '.';
  return out.str();
}


int main() { runTests(); }
