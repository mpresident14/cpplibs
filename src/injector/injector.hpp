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


/************************
 * Type trait templates *
 ************************/

template <typename T>
struct is_unique : false_type {};

template <typename T>
struct is_unique<unique_ptr<T>> {
  using type = T;
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_unique_v = is_unique<T>::value;

template <typename T>
using unique_t = typename is_unique<T>::type;


template <typename T>
struct is_shared : false_type {};

template <typename T>
struct is_shared<shared_ptr<T>> {
  using type = T;
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_shared_v = is_shared<T>::value;

template <typename T>
using shared_t = typename is_shared<T>::type;


/************
 * Concepts *
 ************/

template <typename Ptr>
concept Unique = is_unique_v<Ptr>;

template <typename Ptr>
concept Shared = is_shared_v<Ptr>;

template <typename Ptr>
concept NonPtr = !(Unique<Ptr> || Shared<Ptr>);

// These concepts allow implicit conversion from ptr<Base> to ptr<Derived>, but prevent implicit
// conversion from unique_ptr to shared_ptr
template <typename T, typename Fn>
concept UniqueProvider = is_convertible_v<invoke_result_t<Fn>, unique_ptr<T>>;

template <typename T, typename Fn>
concept SharedProvider =
    is_convertible_v<invoke_result_t<Fn>, shared_ptr<T>> && !UniqueProvider<T, Fn>;

template <typename T, typename Fn>
concept NonPtrProvider = !(UniqueProvider<T, Fn> || SharedProvider<T, Fn>);


/**************
 * Containers *
 **************/

/* Info to be stored in bindings map for some class. */
struct CtorInfo {
  std::vector<std::string> argTypes;
  size_t line;
  const char* filename;
};
enum class BindingType { INSTANCE, UNIQUE_PROVIDER, SHARED_PROVIDER, NON_PTR_PROVIDER };
struct Binding {
  BindingType type;
  std::any obj;
};


/*************
 * Variables *
 *************/

unordered_map<string, Binding> bindings;


/********************
 * Helper Functions *
 ********************/

template <typename T>
constexpr const char* getId() {
  return typeid(decay_t<T>).name();
}

template <typename T, typename R>
void bindToProviderImpl(function<R>&& provider, BindingType bindingType) {
  if (!bindings.emplace(getId<T>(), Binding{ bindingType, any(move(provider)) }).second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throw runtime_error(string("Binding for type ").append(getId<T>()).append(" already exists."));
  }
}

template <typename... Args>
void throwError(Args... msgParts) {
  ostringstream out;
  (..., (out << forward<Args>(msgParts)));
  throw runtime_error(out.str());
}

// TODO: Fix this message
template <typename T>
void wrongBindingError(const char* requestType, const char* boundType) {
  throwError(
      "Request injection of ",
      requestType,
      " of type ",
      getId<T>(),
      ", but only a ",
      boundType,
      " was bound.");
}


}  // namespace


/**********
 * Public *
 **********/

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
    // TODO: Use source::location  in public methods so that we can output line and file of original
    // binding
    throwError("Binding for type ", getId<T>(), " already exists.");
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

template <typename T, typename Fn>
requires NonPtrProvider<T, Fn> void bindToProvider(Fn&& provider) {
  function providerFn = function<T(void)>(forward<Fn>(provider));
  return bindToProviderImpl<T>(move(providerFn), BindingType::NON_PTR_PROVIDER);
}


// TODO: Wrap any_cast exceptions with helpful errors

template <typename Ptr>
requires Unique<Ptr> Ptr inject() {
  using T = unique_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throwError("No binding exists for type ", getId<T>());
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::UNIQUE_PROVIDER:
      return any_cast<function<Ptr(void)>>(binding.obj)();
    case BindingType::INSTANCE:
    case BindingType::SHARED_PROVIDER:
      wrongBindingError<T>("unique_ptr ", "shared_ptr");
    case BindingType::NON_PTR_PROVIDER:
      wrongBindingError<T>("unique_ptr", "object");
  }

  throw runtime_error("Getting rid of clang control path warning.");
}

template <typename Ptr>
requires Shared<Ptr> Ptr inject() {
  using T = shared_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throwError("No binding exists for type ", getId<T>());
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::INSTANCE:
      return any_cast<shared_ptr<T>>(binding.obj);
    case BindingType::SHARED_PROVIDER:
      return any_cast<function<shared_ptr<T>(void)>>(binding.obj)();
    case BindingType::UNIQUE_PROVIDER:
      // Implicit unique->shared ptr okay
      return any_cast<function<unique_ptr<T>(void)>>(binding.obj)();
    case BindingType::NON_PTR_PROVIDER:
      wrongBindingError<T>("shared_ptr", "object");
  }

  throw runtime_error("Getting rid of clang control path warning.");
}

// reference: only instances, no providers because can't have a reference to stack var
template <typename T>
requires(NonPtr<T>&& is_reference_v<T>) T inject() {
  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throwError("No binding exists for type ", getId<T>());
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::INSTANCE:
      return *any_cast<shared_ptr<decay_t<T>>>(binding.obj);
    case BindingType::UNIQUE_PROVIDER:
      wrongBindingError<T>("reference", "unique_ptr");
    case BindingType::SHARED_PROVIDER:
      wrongBindingError<T>("reference", "shared_ptr");
    case BindingType::NON_PTR_PROVIDER:
      wrongBindingError<T>("reference", "object");
  }

  throw runtime_error("Getting rid of clang control path warning.");
}

// not a reference: Only providers of non-ptrs
template <typename T>
requires(NonPtr<T> && !is_reference_v<T>) T inject() {
  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    throwError("No binding exists for type ", getId<T>());
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::INSTANCE:
      wrongBindingError<T>("object", "shared_ptr");
    case BindingType::UNIQUE_PROVIDER:
      wrongBindingError<T>("object", "unique_ptr");
    case BindingType::SHARED_PROVIDER:
      wrongBindingError<T>("object", "shared_ptr");
    case BindingType::NON_PTR_PROVIDER:
      return any_cast<function<T(void)>>(binding.obj)();
  }

  throw runtime_error("Getting rid of clang control path warning.");
}


void clearBindings() { bindings.clear(); }


}  // namespace injector

#endif  // INJECTOR_HPP
