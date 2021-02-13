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
  INJECT(Unrelated(const shared_ptr<string> &s)) : str(*s) {}
  Unrelated(const Unrelated &) = delete;
  Unrelated(Unrelated &&) = default;

  string str;
};

class Base {
public:
  Base() = default;
  Base(int v) : val(v) {}
  virtual ~Base() {}
  Base(const Base &) = delete;
  Base(Base &&) = default;

  int val = BASE;
};

class Derived final : public Base {
public:
  Derived() : Base(DERIVED) {}
  INJECT(Derived(int v, const Unrelated &u)) : Base(v + u.str.size()) {}
  Derived(const Derived &) = delete;
  Derived(Derived &&) = default;
};

struct Annotation1 {};
struct Annotation2 {};

class Child final : public Base {
public:
  ANNOTATED(Annotation1, Annotation2)
  INJECT(Child(char c, const long &lng, unique_ptr<Derived> &&d,
               shared_ptr<Unrelated> u))
      : ch(c), num(lng), derived(std::move(d)), unrelated(std::move(u)) {}
  Child(const Child &) = delete;
  Child(Child &&) = default;

  char ch;
  long num;
  unique_ptr<Derived> derived;
  shared_ptr<Unrelated> unrelated;
};

template <typename T, typename... Types> string errorChain();

BEFORE(setup) { prez::injector::clearBindings(); }

TEST(injectUnique_nonPtrSupplier_throws) {
  prez::injector::bindToSupplier<char>([]() { return CHAR; });
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_uniqueSupplier) {
  prez::injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<Base> b = prez::injector::inject<unique_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectUnique_sharedSupplier_throws) {
  prez::injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<Base>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_sharedObject_throws) {
  prez::injector::bindToObject<Base>(make_shared<Derived>());
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<Base>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_object_throws) {
  prez::injector::bindToObject<char>(CHAR);
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectUnique_impl) {
  prez::injector::bindToObject(NUM);
  prez::injector::bindToSupplier<string>(
      []() { return make_unique<string>(STR); });
  prez::injector::bindToBase<Base, Derived>();
  unique_ptr<Base> b = prez::injector::inject<unique_ptr<Base>>();

  assertEquals(static_cast<int>(NUM + STR.size()), b->val);
}

TEST(injectUnique_byConstructor) {
  prez::injector::bindToObject(NUM);
  prez::injector::bindToSupplier<string>(
      []() { return make_unique<string>(STR); });
  unique_ptr<Base> b = prez::injector::inject<unique_ptr<Derived>>();

  assertEquals(static_cast<int>(NUM + STR.size()), b->val);
}

TEST(injectShared_nonPtrSupplier_throws) {
  prez::injector::bindToSupplier<char>([]() { return CHAR; });
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<char>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectShared_uniqueSupplier) {
  prez::injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<const Base> b = prez::injector::inject<unique_ptr<const Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectShared_sharedSupplier) {
  prez::injector::bindToSupplier<Base>([]() { return make_shared<Derived>(); });
  shared_ptr<Base> b = prez::injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectShared_sharedObject) {
  prez::injector::bindToObject(make_shared<Base>());
  shared_ptr<Base> b = prez::injector::inject<shared_ptr<Base>>();

  assertEquals(BASE, b->val);
}

TEST(injectShared_object_throws) {
  prez::injector::bindToObject(NUM);
  string err =
      assertThrows([]() { prez::injector::inject<shared_ptr<int>>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectShared_impl) {
  prez::injector::bindToSupplier<Derived>(
      []() { return make_shared<Derived>(); });
  prez::injector::bindToBase<Base, Derived>();
  shared_ptr<Base> b = prez::injector::inject<shared_ptr<Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectShared_byConstructor) {
  prez::injector::bindToObject(NUM);
  prez::injector::bindToSupplier<string>(
      []() { return make_unique<string>(STR); });
  shared_ptr<Base> b = prez::injector::inject<shared_ptr<Derived>>();

  assertEquals(static_cast<int>(NUM + STR.size()), b->val);
}

TEST(injectNonPtr_nonPtrSupplier) {
  prez::injector::bindToSupplier<Base>([]() { return Derived(); });
  const Base &b = prez::injector::inject<Base>();

  assertEquals(DERIVED, b.val);
}

TEST(injectNonPtr_uniqueSupplier_throws) {
  prez::injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  string err = assertThrows([]() { prez::injector::inject<Base>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_sharedSupplier_throws) {
  prez::injector::bindToSupplier<Base>([]() { return make_shared<Base>(); });
  string err = assertThrows([]() { prez::injector::inject<Base>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_sharedObject_throws) {
  auto ptr = make_shared<Derived>();

  prez::injector::bindToObject<Derived>(ptr);
  string err = assertThrows([]() { prez::injector::inject<Derived>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_object) {
  prez::injector::bindToObject(NUM);
  int n = prez::injector::inject<int>();

  assertEquals(NUM, n);
}

TEST(injectNonPtr_impl_throws) {
  prez::injector::bindToObject(NUM);
  prez::injector::bindToSupplier<string>(
      []() { return make_unique<string>(STR); });
  prez::injector::bindToBase<Base, Derived>();
  string err = assertThrows([]() { prez::injector::inject<Base>(); });

  assertContains("Incompatible binding", err);
}

TEST(injectNonPtr_byConstructor) {
  prez::injector::bindToObject(NUM);
  prez::injector::bindToSupplier<string>(
      []() { return make_unique<string>(STR); });
  Base b = prez::injector::inject<Derived>();

  assertEquals(static_cast<int>(NUM + STR.size()), b.val);
}

TEST(injectConst_constBinding) {
  prez::injector::bindToObject<const Base>(make_shared<const Derived>());
  shared_ptr<const Base> b = prez::injector::inject<shared_ptr<const Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectConst_nonconstBinding) {
  prez::injector::bindToSupplier<Base>([]() { return make_unique<Derived>(); });
  unique_ptr<const Base> b = prez::injector::inject<unique_ptr<const Base>>();

  assertEquals(DERIVED, b->val);
}

TEST(injectNonConst_constBinding_throws) {
  prez::injector::bindToSupplier<const Base>(
      []() { return make_unique<Derived>(); });
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<Base>>(); });

  assertContains("Incompatible binding for type", err);
}

TEST(inject_withAnnotations) {
  prez::injector::bindToObject<int, Annotation1>(NUM);

  assertEquals(NUM, prez::injector::inject<int, Annotation1>());
}

TEST(inject_byConstructorWithAnnotations) {
  int m = 2780725;

  prez::injector::bindToObject<char, Annotation1>(CHAR);
  prez::injector::bindToObject<long, Annotation2>(m);
  prez::injector::bindToSupplier<int>([]() { return NUM; });
  prez::injector::bindToSupplier<string>(
      []() { return make_shared<string>(STR); });
  prez::injector::bindToBase<Base, Child>();
  shared_ptr<Base> base = prez::injector::inject<shared_ptr<Base>>();

  shared_ptr<Child> child = std::static_pointer_cast<Child>(base);
  Derived &d = *child->derived;
  shared_ptr<Unrelated> &u = child->unrelated;
  assertEquals(static_cast<int>(NUM + STR.size()), d.val);
  assertEquals(u->str, STR);
  assertEquals(CHAR, child->ch);
  assertEquals(m, child->num);
}

TEST(inject_noBinding_throws) {
  string err =
      assertThrows([]() { prez::injector::inject<shared_ptr<Base>>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_wrongBinding_throws) {
  prez::injector::bindToSupplier<int>([]() { return make_shared<int>(NUM); });
  string err = assertThrows([]() { prez::injector::inject<string>(); });

  assertContains("not bound and has no constructors", err);
}

TEST(inject_noBindingNested_throwsWithCorrectInjectionChain) {
  prez::injector::bindToSupplier<int>([]() { return NUM; });
  string err = assertThrows([]() { prez::injector::inject<Derived>(); });

  assertContains(errorChain<Derived, Unrelated, string>(), err);
}

TEST(inject_noBinding_classBinding_throwsWithCorrectInjectionChain) {
  prez::injector::bindToSupplier<string>([]() { return STR; });
  prez::injector::bindToBase<Base, Derived>();
  string err =
      assertThrows([]() { prez::injector::inject<unique_ptr<Base>>(); });

  assertContains(errorChain<Base, Derived, int>(), err);
}

TEST(inject_noAnnotatedBinding_throwsWithCorrectInjectionChain) {
  std::ostringstream expectedErr;
  expectedErr << "Injection chain:\n\t" << typeid(Derived).name()
              << " (annotated with " << typeid(Annotation1).name() << ") -> "
              << typeid(int).name() << '.';

  string err =
      assertThrows([]() { prez::injector::inject<Derived, Annotation1>(); });

  assertContains(expectedErr.str(), err);
}

TEST(bind_multiple_throws) {
  prez::injector::bindToSupplier<int>([]() { return make_unique<int>(NUM); });
  string err = assertThrows(
      []() { prez::injector::bindToObject<int>(make_shared<int>(23)); });

  assertContains("already exists", err);
}

template <typename T, typename... Ts> string errorChain() {
  std::ostringstream out;
  out << "Injection chain:\n\t" << typeid(T).name();
  (..., (out << " -> " << typeid(Ts).name()));
  out << '.';
  return out.str();
}

int main() { return runTests(); }
