#include "src/injector/binding_map.hpp"

#include "src/injector/util.hpp"
#include "src/testing/unit_test.hpp"

#include <any>
#include <sstream>
#include <string>

#include <experimental/source_location>


using namespace std;
using namespace unit_test;
namespace injdet = injector::detail;


injdet::BindingMap bindingMap;

struct Annotation1 {};
struct Annotation2 {};
static const char* TYPE_ID_0 = injdet::getId<int>();
static const char* TYPE_ID_1 = injdet::getId<char>();
static const char* TYPE_ID_2 = injdet::getId<long>();
static constexpr int OBJ_0 = 5;
static constexpr int OBJ_1 = 10;
static constexpr int OBJ_2 = 99;
static const std::experimental::source_location LOC = std::experimental::source_location::current();
static const injdet::BindingType BINDING_TYPE = injdet::BindingType::SHARED;


BEFORE(setup) { bindingMap.clearBindings(); }


TEST(insertDefault_success) {
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_0);

  assertEquals(BINDING_TYPE, binding->type);
  assertEquals(OBJ_0, any_cast<int>(binding->obj));
}

TEST(insertAnnotated_success) {
  bindingMap.insertBinding<Annotation1>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding<Annotation1>(TYPE_ID_0);

  assertEquals(BINDING_TYPE, binding->type);
  assertEquals(OBJ_0, any_cast<int>(binding->obj));
}

TEST(insertMultipleDifferentAnnotations_success) {
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding<Annotation1>(TYPE_ID_0, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding<Annotation2>(TYPE_ID_0, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_0);
  injdet::Binding* binding1 = bindingMap.lookupBinding<Annotation1>(TYPE_ID_0);
  injdet::Binding* binding2 = bindingMap.lookupBinding<Annotation2>(TYPE_ID_0);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertMultipleDifferentTypeDefault_success) {
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_1, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_2, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_0);
  injdet::Binding* binding1 = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_1);
  injdet::Binding* binding2 = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_2);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertMultipleDifferentTypeAnnotated_success) {
  bindingMap.insertBinding<Annotation1>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding<Annotation1>(TYPE_ID_1, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding<Annotation1>(TYPE_ID_2, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding<Annotation1>(TYPE_ID_0);
  injdet::Binding* binding1 = bindingMap.lookupBinding<Annotation1>(TYPE_ID_1);
  injdet::Binding* binding2 = bindingMap.lookupBinding<Annotation1>(TYPE_ID_2);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertDuplicateDefault_throws) {
  int n = 8;

  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  string err = assertThrows([n]() {
    bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(n), BINDING_TYPE, LOC);
  });

  assertContains("Unannotated binding for type", err);
}

TEST(insertDuplicateAnnotated_throws) {
  bindingMap.insertBinding<Annotation1>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  string err = assertThrows(
      []() { bindingMap.insertBinding<Annotation1>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC); });

  assertContains("Binding for type", err);
}

TEST(lookupUnbound_returnsNullptr) {
  injdet::Binding* binding = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_0);

  assertEquals(nullptr, binding);
}

TEST(lookupDifferentType_returnsNullptr) {
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding<injector::DefaultAnnotation>(TYPE_ID_1);

  assertEquals(nullptr, binding);
}

TEST(lookupDifferentAnnotation_returnsNullptr) {
  bindingMap.insertBinding<injector::DefaultAnnotation>(TYPE_ID_0, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding<Annotation1>(TYPE_ID_0);

  assertEquals(nullptr, binding);
}

int main() { runTests(); }
