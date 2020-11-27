#include "src/injector/injector.hpp"

#include "src/testing/unit_test.hpp"

#include <memory>
#include <sstream>
#include <string>

using namespace std;
using namespace unit_test;


static int BASE = 14;
static int DERIVED = 999;
static constexpr int NUM = 12345;
static constexpr char CHAR = 'c';
static string STR = "string";

class Unrelated final {
public:
  INJECT(Unrelated(shared_ptr<string> s)) : str(*s) {}
  Unrelated(const Unrelated&) = delete;
  Unrelated(Unrelated&&) = default;

  string str;
};

class Base {
public:
  Base() = default;
  Base(int v) : val(v) {}
  Base(const Base&) = delete;
  Base(Base&&) = default;

  int val = BASE;
};

class Derived final : public Base {
public:
  Derived() : Base(DERIVED) {}
  INJECT(Derived(int v, const Unrelated& u)) : Base(v + u.str.size()) {}
  Derived(const Derived&) = delete;
  Derived(Derived&&) = default;
};

struct Annotation1 {};
struct Annotation2 {};

class Child final : public Base {
public:
  ANNOTATED(Annotation1, Annotation2)
  INJECT(Child(char c, const long& lng, unique_ptr<Derived>&& d, shared_ptr<Unrelated> u))
      : ch(c), num(lng), derived(std::move(d)), unrelated(std::move(u)) {}
  Child(const Child&) = delete;
  Child(Child&&) = default;

  char ch;
  long num;
  unique_ptr<Derived> derived;
  shared_ptr<Unrelated> unrelated;
};

template <typename T, typename... Types>
string errorChain();


BEFORE(setup) { injector::clearBindings(); }


TEST(injectUnique_nonPtrSupplier) {
  injector::bindToSupplier<char>([]() { return CHAR; });
  string err = assertThrows([]() { injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectUnique_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  string err = assertThrows([]() { injector::inject<unique_ptr<Base>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_sharedObject) {
  injector::bindToObject<Base>(make_shared<Derived>());
  string err = assertThrows([]() { injector::inject<unique_ptr<Base>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_object) {
  injector::bindToObject<char>(CHAR);
  string err = assertThrows([]() { injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_impl) {
  injector::bindToObject(NUM);
  injector::bindToSupplier<string>([]() { return make_unique<string>(STR); });
  injector::bindToClass<Base, Derived>();
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(NUM + STR.size(), b->val);
}

TEST(injectUnique_byConstructor) {
  injector::bindToObject(NUM);
  injector::bindToSupplier<string>([]() { return make_unique<string>(STR); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Derived>>();

  assertEquals(NUM + STR.size(), b->val);
}

TEST(injectShared_nonPtrSupplier) {
  injector::bindToSupplier<char>([]() { return CHAR; });
  string err = assertThrows([]() { injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectShared_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectShared_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectShared_sharedObject) {
  injector::bindToObject(make_shared<Base>());
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
}

TEST(injectShared_object) {
  injector::bindToObject(NUM);
  string err = assertThrows([]() { injector::inject<shared_ptr<int>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectShared_impl) {
  injector::bindToSupplier<Derived>([]() { return make_shared<Derived>(); });
  injector::bindToClass<Base, Derived>();
  shared_ptr<Base> b = injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}


TEST(injectShared_byConstructor) {
  injector::bindToObject(NUM);
  injector::bindToSupplier<string>([]() { return make_unique<string>(STR); });
  shared_ptr<Base> b = injector::inject<shared_ptr<Derived>>();

  assertEquals(NUM + STR.size(), b->val);
}

TEST(injectNonPtr_nonPtrSupplier) {
  injector::bindToSupplier<Base>([]() { return Derived(); });
  const Base& b = injector::inject<Base>();

  assertEquals(DERIVED, b.val);
}

TEST(injectNonPtr_uniqueSupplier) {
  injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  string err = assertThrows([]() { injector::inject<Base>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_sharedSupplier) {
  injector::bindToSupplier<Base>([]() { return make_shared<Base>(); });
  string err = assertThrows([]() { injector::inject<Base>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_sharedObject) {
  auto ptr = make_shared<Derived>();

  injector::bindToObject<Derived>(ptr);
  string err = assertThrows([]() { injector::inject<Derived>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_object) {
  injector::bindToObject(NUM);
  int n = injector::inject<int>();

  assertEquals(NUM, n);
}

TEST(injectNonPtr_impl) {
  injector::bindToObject(NUM);
  injector::bindToSupplier<string>([]() { return make_unique<string>(STR); });
  injector::bindToClass<Base, Derived>();
  Base b = injector::inject<Base>();

  assertEquals(NUM + STR.size(), b.val);
}


TEST(injectNonPtr_byConstructor) {
  injector::bindToObject(NUM);
  injector::bindToSupplier<string>([]() { return make_unique<string>(STR); });
  Base b = injector::inject<Derived>();

  assertEquals(NUM + STR.size(), b.val);
}

TEST(inject_withAnnotations) {
  injector::bindToObject<Annotation1>(NUM);

  assertEquals(NUM, injector::inject<int, Annotation1>());
}


TEST(inject_byConstructorWithAnnotations) {
  int m = 2780725;

  injector::bindToObject<Annotation1>(CHAR);
  injector::bindToObject<long, Annotation2>(m);
  injector::bindToSupplier<int>([]() { return NUM; });
  injector::bindToSupplier<string>([]() { return make_shared<string>(STR); });
  injector::bindToClass<Base, Child>();
  shared_ptr<Base> base = injector::inject<shared_ptr<Base>>();

  shared_ptr<Child> child = std::static_pointer_cast<Child>(base);
  Derived& d = *child->derived;
  shared_ptr<Unrelated>& u = child->unrelated;
  assertEquals(NUM + STR.size(), d.val);
  assertEquals(u->str, STR);
  assertEquals(CHAR, child->ch);
  assertEquals(m, child->num);
}

TEST(inject_noBinding_throws) {
  string err = assertThrows([]() { injector::inject<shared_ptr<Base>>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_wrongBinding_throws) {
  injector::bindToSupplier<int>([]() { return make_shared<int>(NUM); });
  string err = assertThrows([]() { injector::inject<string>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_noBindingNested_throwsWithCorrectInjectionChain) {
  injector::bindToSupplier<int>([]() { return NUM; });
  string err = assertThrows([]() { injector::inject<Derived>(); });

  assertContains(errorChain<Derived, Unrelated, string>(), err);
}

TEST(inject_noBinding_classBinding_throwsWithCorrectInjectionChain) {
  injector::bindToSupplier<string>([]() { return STR; });
  injector::bindToClass<Base, Derived>();
  string err = assertThrows([]() { injector::inject<Base>(); });

  assertContains(errorChain<Base, Derived, int>(), err);
}

TEST(inject_noAnnotatedBinding_throwsWithCorrectInjectionChain) {
  std::ostringstream expectedErr;
  expectedErr << "Injection chain:\n\t" << typeid(Derived).name() << " (annotated with "
              << typeid(Annotation1).name() << ") -> " << typeid(int).name() << '.';

  string err = assertThrows([]() { injector::inject<Derived, Annotation1>(); });

  assertContains(expectedErr.str(), err);
}

TEST(bind_multiple_throws) {
  injector::bindToSupplier<int>([]() { return make_unique<int>(NUM); });
  string err = assertThrows([]() { injector::bindToObject<int>(make_shared<int>(23)); });

  assertContains("already exists", err);
}

template <typename T, typename... Ts>
string errorChain() {
  std::ostringstream out;
  out << "Injection chain:\n\t" << typeid(T).name();
  (..., (out << " -> " << typeid(Ts).name()));
  out << '.';
  return out.str();
}


int main() { runTests(); }
