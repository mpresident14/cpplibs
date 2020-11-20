#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
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

// TODO: Seems to be applying within injector namespace as well
using namespace std;

const char CTRL_PATH[] = "Getting rid of clang control path warning.";

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


/* Check to see if class has an injected constructor. */
template <typename C>
struct has_injected_ctor {
  template <typename T>
  static auto test(typename T::InjectCtor) -> std::true_type;

  template <typename T>
  static auto test(...) -> std::false_type;

  // Passing in nullptr tries the pointer parameter overload first.
  static constexpr bool value = decltype(has_injected_ctor::test<C>(nullptr))::value;
};

template <typename T>
constexpr bool has_injected_ctor_v = has_injected_ctor<T>::value;

/************
 * Concepts *
 ************/

// TODO: Remove Unique and Shared, change NonPtr to non_ptr_v
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
      "Requested injection of ",
      requestType,
      " of type ",
      getId<T>(),
      ", but found ",
      boundType,
      " binding.");
}

template <typename R, typename... Args>
struct is_ctor_of_final : false_type {};

template <typename R, typename... Args>
struct is_ctor_of_final<R(Args...)> {
  static constexpr bool value = is_final_v<R>;
};

template <typename F>
constexpr bool is_ctor_of_final_v = is_ctor_of_final<F>::value;

template <typename R, typename... Args>
struct CtorInvoker {};

template <typename R, typename... Args>
 struct CtorInvoker<R(Args...)> {
  static unique_ptr<R> invokeUnique();
  static shared_ptr<R> invokeShared();
  static R invokeNonPtr();
};

// TODO: inject should automatically try this if binding doesn't exist (use has_injected_ctor_v to
// change injection)
// TODO: Enforce that class must be final if it has an injected constructor (otherwise, derived
// classes inherited injected constructor and compiler will fail on attempt to convert Base to
// Derived).
template <typename T, typename R = unique_t<T>>
requires Unique<T>&& has_injected_ctor_v<R> && is_final_v<R> T injectByConstructor() {
  return CtorInvoker<typename R::InjectCtor>::invokeUnique();
}

template <typename T, typename R = unique_t<T>>
    requires Unique<T> && (!has_injected_ctor_v<R>)T injectByConstructor() {
  throwError("Class ", getId<R>(), " has no constructors for injection.");
  throw runtime_error(CTRL_PATH);
}

template <typename T, typename R = shared_t<T>>
requires Shared<T>&& has_injected_ctor_v<R> && is_final_v<R> T injectByConstructor() {
  return CtorInvoker<typename R::InjectCtor>::invokeShared();
}

template <typename T, typename R = shared_t<T>>
    requires Shared<T> && (!has_injected_ctor_v<R>)T injectByConstructor() {
  throwError("Class ", getId<R>(), " has no constructors for injection.");
  throw runtime_error(CTRL_PATH);
}

template <typename T, typename R = decay_t<T>>
requires NonPtr<T>&& has_injected_ctor_v<R> && is_final_v<R> T injectByConstructor() {
  return CtorInvoker<typename R::InjectCtor>::invokeNonPtr();
}

template <typename T, typename R = decay_t<T>>
    requires NonPtr<T> && (!has_injected_ctor_v<R>)T injectByConstructor() {
  throwError("Class ", getId<R>(), " has no constructors for injection.");
  throw runtime_error(CTRL_PATH);
}

}  // namespace


/**********
 * Public *
 **********/

namespace injector {

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

// Inject a unique ptr finds bindings of:
// -non-ptr providers
// +unique ptr providers
// -shared_ptr providers
// -instances
template <typename Ptr>
requires Unique<Ptr> Ptr inject() {
  using T = unique_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    // TODO: Record chain of injection invocations for debugging
    return injectByConstructor<Ptr>();
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::NON_PTR_PROVIDER:
      wrongBindingError<T>("unique_ptr", "object");
    case BindingType::UNIQUE_PROVIDER:
      return any_cast<function<Ptr(void)>>(binding.obj)();
    case BindingType::SHARED_PROVIDER:
    case BindingType::INSTANCE:
      wrongBindingError<T>("unique_ptr ", "shared_ptr");
  }

  throw runtime_error(CTRL_PATH);
}

// Inject a shared ptr finds bindings of:
// -non-ptr providers
// +unique ptr providers
// +shared_ptr providers
// +instances
template <typename Ptr>
requires Shared<Ptr> Ptr inject() {
  using T = shared_t<Ptr>;

  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    return injectByConstructor<Ptr>();
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::NON_PTR_PROVIDER:
      wrongBindingError<T>("shared_ptr", "object");
    case BindingType::UNIQUE_PROVIDER:
      // Implicit unique->shared ptr okay
      return any_cast<function<unique_ptr<T>(void)>>(binding.obj)();
    case BindingType::SHARED_PROVIDER:
      return any_cast<function<shared_ptr<T>(void)>>(binding.obj)();
    case BindingType::INSTANCE:
      return any_cast<shared_ptr<T>>(binding.obj);
  }

  throw runtime_error(CTRL_PATH);
}

// Inject a const-reference finds bindings of:
// +non-ptr providers
// +unique ptr providers
// +shared_ptr providers
// +instances
template <typename T>
requires NonPtr<T> T inject() {
  using decT = decay_t<T>;
  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    return injectByConstructor<T>();
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::NON_PTR_PROVIDER:
      return any_cast<function<decT(void)>>(binding.obj)();
    case BindingType::UNIQUE_PROVIDER:
      return move(*any_cast<function<unique_ptr<decT>(void)>>(binding.obj)());
    case BindingType::SHARED_PROVIDER:
      return move(*any_cast<function<shared_ptr<decT>(void)>>(binding.obj)());
    case BindingType::INSTANCE:
      return *any_cast<shared_ptr<decT>>(binding.obj);
  }

  throw runtime_error(CTRL_PATH);
}


void clearBindings() { bindings.clear(); }

}  // namespace injector

namespace {


template <typename R, typename... Args>
 unique_ptr<R> CtorInvoker<R(Args...)>::invokeUnique() {
  return make_unique<R>(injector::inject<Args>()...);
}

template <typename R, typename... Args>
 shared_ptr<R> CtorInvoker<R(Args...)>::invokeShared() {
  return make_shared<R>(injector::inject<Args>()...);
}

template <typename R, typename... Args>
 R CtorInvoker<R(Args...)>::invokeNonPtr() {
  return R(injector::inject<Args>()...);
}

}  // namespace

#endif  // INJECTOR_HPP
