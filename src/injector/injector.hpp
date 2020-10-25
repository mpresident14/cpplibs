#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)                                                    \
  int injector_dummy_var = injector::addCtor<ctorDecl>(__LINE__, __FILE__); \
  ctorDecl

#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <prez/print_stuff.hpp>

namespace {

using namespace std;

/* Templates to extract the class type and parameter types from a constructor. */
template <typename R, typename... Args>
struct CtorTypes;

template <typename R, typename... Args>
struct CtorTypes<R(Args...)> {
  string classType = typeid(R).name();
  vector<string> argTypes = { typeid(Args).name()... };
};

/* Info to be stored in bindings map for some class. */
struct CtorInfo {
  std::vector<std::string> argTypes;
  size_t line;
  const char* filename;
};
unordered_map<string, CtorInfo> constructors;

enum class BindingType { INSTANCE, INSTANCE_PTR, CLASS, PROVIDER };
struct Binding {
  BindingType type;
  std::any obj;
};
unordered_map<string, Binding> bindings;

template <typename T>
const char* getId() {
  return typeid(decay_t<T>).name();
}

}  // namespace


namespace injector {

/* Add constructor type info to the constructors map. */
template <typename F>
int addCtor(size_t line, const char* filename) {
  CtorTypes<F> ctorTypes;
  auto p = constructors.emplace(
      move(ctorTypes.classType), CtorInfo{ move(ctorTypes.argTypes), line, filename });
  if (!p.second) {
    throw runtime_error(
        "INTERNAL ERROR: Should not reach here because injector_dummy_var should have been "
        "redefined within class "
        + ctorTypes.classType);
  }

  return 0;
}

template <typename Bound, typename T>
    requires same_as<Bound, T> || derived_from<Bound, T> void bindToInstance(T&& obj) {
  if (!bindings.emplace(getId<T>(), Binding{ BindingType::INSTANCE, make_any<T>(forward<T>(obj)) })
           .second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throw runtime_error(string("Binding for type ").append(getId<T>()).append(" already exists."));
  }
}

// std::any requires a copy-constructible type, so we offer bindToInstancePtr for
// non-copy-constructible types
template <typename T>
requires is_copy_constructible_v<T> void bindToInstance(T&& obj) {
  return bindToInstance<T, T>(forward<T>(obj));
}

template <typename Bound, typename T>
    requires same_as<Bound, T> || derived_from<Bound, T> void bindToInstancePtr(unique_ptr<T>&& objPtr) {
  if (!bindings
           .emplace(
               getId<T>(),
               Binding{ BindingType::INSTANCE_PTR,
                        make_any<shared_ptr<T>>(shared_ptr<T>(objPtr.release())) })
           .second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throw runtime_error(string("Binding for type ").append(getId<T>()).append(" already exists."));
  }
}

template <typename T>
void bindToInstancePtr(unique_ptr<T>&& objPtr) {
  return bindToInstancePtr<T, T>(move(objPtr));
}

template <typename T>
T& inject() {
  auto iter = bindings.find(typeid(T).name());
  if (iter == bindings.end()) {
    throw runtime_error(string("No binding exists for type ").append(typeid(T).name()));
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::INSTANCE:
      return any_cast<T&>(binding.obj);
    case BindingType::INSTANCE_PTR:
      return *any_cast<shared_ptr<T>>(binding.obj);
    default:
      throw runtime_error("Unimplemented");
  }
}

}  // namespace injector


// TODO: Problem with injecting a raw ptr into ctor is that class doesn't know whether it those ptrs
// were injected and need to be deleted.

#endif  // INJECTOR_HPP
