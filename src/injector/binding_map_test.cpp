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
const char* TYPE_ID_0 = injdet::getId<int>();
const char* TYPE_ID_1 = injdet::getId<char>();
const char* TYPE_ID_2 = injdet::getId<long>();
const char* DEFAULT_ANNOTATION = injdet::getId<injdet::DefaultAnnotation>();
const char* OTHER_ANNOTATION_1 = injdet::getId<char>();
const char* OTHER_ANNOTATION_2 = injdet::getId<long>();
int OBJ_0 = 5;
int OBJ_1 = 10;
int OBJ_2 = 99;
std::experimental::source_location LOC = std::experimental::source_location::current();
injdet::BindingType BINDING_TYPE = injdet::BindingType::SHARED;


BEFORE(setup) { bindingMap.clearBindings(); }


TEST(insertDefault_success) {
  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding(TYPE_ID_0, DEFAULT_ANNOTATION);

  assertEquals(BINDING_TYPE, binding->type);
  assertEquals(OBJ_0, any_cast<int>(binding->obj));
}

TEST(insertAnnotated_success) {
  bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_1, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding(TYPE_ID_0, OTHER_ANNOTATION_1);

  assertEquals(BINDING_TYPE, binding->type);
  assertEquals(OBJ_0, any_cast<int>(binding->obj));
}

TEST(insertMultipleDifferentAnnotations_success) {
  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_1, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_2, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding(TYPE_ID_0, DEFAULT_ANNOTATION);
  injdet::Binding* binding1 = bindingMap.lookupBinding(TYPE_ID_0, OTHER_ANNOTATION_1);
  injdet::Binding* binding2 = bindingMap.lookupBinding(TYPE_ID_0, OTHER_ANNOTATION_2);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertMultipleDifferentTypeDefault_success) {
  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_1, DEFAULT_ANNOTATION, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_2, DEFAULT_ANNOTATION, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding(TYPE_ID_0, DEFAULT_ANNOTATION);
  injdet::Binding* binding1 = bindingMap.lookupBinding(TYPE_ID_1, DEFAULT_ANNOTATION);
  injdet::Binding* binding2 = bindingMap.lookupBinding(TYPE_ID_2, DEFAULT_ANNOTATION);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertMultipleDifferentTypeAnnotated_success) {
  bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_1, any(OBJ_0), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_1, OTHER_ANNOTATION_1, any(OBJ_1), BINDING_TYPE, LOC);
  bindingMap.insertBinding(TYPE_ID_2, OTHER_ANNOTATION_1, any(OBJ_2), BINDING_TYPE, LOC);
  injdet::Binding* binding0 = bindingMap.lookupBinding(TYPE_ID_0, OTHER_ANNOTATION_1);
  injdet::Binding* binding1 = bindingMap.lookupBinding(TYPE_ID_1, OTHER_ANNOTATION_1);
  injdet::Binding* binding2 = bindingMap.lookupBinding(TYPE_ID_2, OTHER_ANNOTATION_1);

  assertEquals(BINDING_TYPE, binding0->type);
  assertEquals(BINDING_TYPE, binding1->type);
  assertEquals(BINDING_TYPE, binding2->type);
  assertEquals(OBJ_0, any_cast<int>(binding0->obj));
  assertEquals(OBJ_1, any_cast<int>(binding1->obj));
  assertEquals(OBJ_2, any_cast<int>(binding2->obj));
}

TEST(insertDuplicateDefault_throws) {
  int n = 8;

  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  string err = assertThrows([n]() {
    bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(n), BINDING_TYPE, LOC);
  });

  assertContains("Unannotated binding for type", err);
}

TEST(insertDuplicateAnnotated_throws) {
  bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_2, any(OBJ_0), BINDING_TYPE, LOC);
  string err = assertThrows([]() {
    bindingMap.insertBinding(TYPE_ID_0, OTHER_ANNOTATION_2, any(OBJ_0), BINDING_TYPE, LOC);
  });

  assertContains("Binding for type", err);
}

TEST(lookupUnbound_returnsNullptr) {
  injdet::Binding* binding = bindingMap.lookupBinding(TYPE_ID_0, DEFAULT_ANNOTATION);

  assertEquals(nullptr, binding);
}

TEST(lookupDifferentType_returnsNullptr) {
  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding(TYPE_ID_1, DEFAULT_ANNOTATION);

  assertEquals(nullptr, binding);
}

TEST(lookupDifferentAnnotation_returnsNullptr) {
  bindingMap.insertBinding(TYPE_ID_0, DEFAULT_ANNOTATION, any(OBJ_0), BINDING_TYPE, LOC);
  injdet::Binding* binding = bindingMap.lookupBinding(TYPE_ID_0, OTHER_ANNOTATION_1);

  assertEquals(nullptr, binding);
}

int main() { runTests(); }
