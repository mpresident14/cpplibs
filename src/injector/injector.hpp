#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)                                      \
  using InjectCtor = injector::CtorTypes<__LINE__, ctorDecl>; \
  ctorDecl

#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <prez/print_stuff.hpp>

namespace {

using namespace std;


template <typename T>
struct is_uniq_ptr : false_type {};

template <typename T>
struct is_uniq_ptr<unique_ptr<T>> {
  using type = T;
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_uniq_ptr_v = is_uniq_ptr<T>::value;

template <typename T>
using uniq_ptr_t = typename is_uniq_ptr<T>::type;


template <typename T>
struct is_shared_ptr : false_type {};

template <typename T>
struct is_shared_ptr<shared_ptr<T>> {
  using type = T;
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T>
using shared_ptr_t = typename is_shared_ptr<T>::type;

// Templates allow implicit conversion from ptr<Base> to ptr<Derived>, but prevent implicit
// conversion from unique_ptr to shared_ptr
template <typename T, typename Fn>
concept UniqueProvider = is_convertible_v<invoke_result_t<Fn>, unique_ptr<T>>;

template <typename T, typename Fn>
concept SharedProvider =
    is_convertible_v<
        invoke_result_t<Fn>,
        shared_ptr<T>> && !is_convertible_v<invoke_result_t<Fn>, unique_ptr<T>>;

/* Info to be stored in bindings map for some class. */
struct CtorInfo {
  std::vector<std::string> argTypes;
  size_t line;
  const char* filename;
};
unordered_map<string, CtorInfo> constructors;

enum class BindingType { INSTANCE, CLASS, UNIQUE_PROVIDER, SHARED_PROVIDER };
struct Binding {
  BindingType type;
  std::any obj;
};
unordered_map<string, Binding> bindings;

template <typename T>
const char* getId() {
  return typeid(decay_t<T>).name();
}

template <typename T, typename R>
void bindToProviderImpl(function<R>&& provider, BindingType bindingType) {
  if (!bindings.emplace(getId<T>(), Binding{ bindingType, any(move(provider)) }).second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throw runtime_error(string("Binding for type ").append(getId<T>()).append(" already exists."));
  }
}

}  // namespace


namespace injector {

/* Templates to extract the class type and parameter types from a constructor. */
template <size_t N, typename R, typename... Args>
struct CtorTypes;

template <size_t N, typename R, typename... Args>
struct CtorTypes<N, R(Args...)> {
  string classType = typeid(R).name();
  vector<string> argTypes = { typeid(Args).name()... };
};

template <typename Bound, typename T>
    requires same_as<Bound, T> || derived_from<Bound, T> void bindToInstance(shared_ptr<T> objPtr) {
  if (!bindings.emplace(getId<T>(), Binding{ BindingType::INSTANCE, any(move(objPtr)) }).second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throw runtime_error(string("Binding for type ").append(getId<T>()).append(" already exists."));
  }
}

template <typename T>
void bindToInstance(shared_ptr<T> objPtr) {
  return bindToInstance<T, T>(move(objPtr));
}


template <typename T, typename Fn>
requires UniqueProvider<T, Fn> void bindToProvider(Fn&& provider) {
  function providerFn = function<unique_ptr<T>(void)>(forward<Fn>(provider));
  return bindToProviderImpl<T>(move(providerFn), BindingType::UNIQUE_PROVIDER);
}

template <typename T, typename Fn>
requires SharedProvider<T, Fn> void bindToProvider(Fn&& provider) {
  function providerFn = function<shared_ptr<T>(void)>(forward<Fn>(provider));
  return bindToProviderImpl<T>(move(providerFn), BindingType::SHARED_PROVIDER);
}


template <typename Ptr>
requires is_uniq_ptr_v<Ptr> Ptr inject() {
  using T = uniq_ptr_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throw runtime_error(string("No binding exists for type ").append(getId<T>()));
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::UNIQUE_PROVIDER:
      return any_cast<function<Ptr(void)>>(binding.obj)();
    case BindingType::INSTANCE:
    case BindingType::SHARED_PROVIDER:
      throw runtime_error(
          string("A unique_ptr of type ")
              .append(getId<T>())
              .append(" was requested for injection, but only a shared_ptr was bound."));
    default:
      throw runtime_error("Unimplemented");
  }
}

template <typename Ptr>
requires is_shared_ptr_v<Ptr> Ptr inject() {
  using T = shared_ptr_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throw runtime_error(string("No binding exists for type ").append(getId<T>()));
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::INSTANCE:
      return any_cast<Ptr>(binding.obj);
    case BindingType::SHARED_PROVIDER:
      return any_cast<function<Ptr(void)>>(binding.obj)();
    case BindingType::UNIQUE_PROVIDER:
      // Implicit unique->shared ptr okay
      return any_cast<function<unique_ptr<T>(void)>>(binding.obj)();
    default:
      throw runtime_error("Unimplemented");
  }
}


void clearBindings() { bindings.clear(); }


}  // namespace injector

#endif  // INJECTOR_HPP
