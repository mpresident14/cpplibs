#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
using unique_t = typename is_unique<T>::type;


template <typename T>
struct is_shared : false_type {};

template <typename T>
struct is_shared<shared_ptr<T>> {
  using type = T;
  static constexpr bool value = true;
};

template <typename T>
using shared_t = typename is_shared<T>::type;

/************
 * Concepts *
 ************/

template <typename Ptr>
concept Unique = is_unique<Ptr>::value;

template <typename Ptr>
concept Shared = is_shared<Ptr>::value;

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
// struct CtorInfo {
//   std::vector<std::string> argTypes;
//   size_t line;
//   const char* filename;
// };
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

template <typename T, typename R>
void bindToProviderImpl(function<R>&& provider, BindingType bindingType) {
  if (!bindings.emplace(getId<T>(), Binding{ bindingType, any(move(provider)) }).second) {
    // TODO: Maybe use macro so that we can output line and file of original binding
    throwError("Binding for type ", getId<T>(), " already exists.");
  }
}

template <typename C>
concept HasInjectCtor = requires {
  typename C::InjectCtor;
};

template <typename T>
struct type_extractor {
  using type =
      conditional_t<Unique<T>, unique_t<T>, conditional_t<Shared<T>, shared_t<T>, decay_t<T>>>;
};
template <typename T>
using type_extractor_t = typename type_extractor<T>::type;

template <typename R, typename... Args>
struct CtorInvoker {};

template <typename R, typename... Args>
struct CtorInvoker<R(Args...)> {
  template <typename T>
  static T invoke();
  template <typename T>
  requires Unique<T> static T invokeImpl();
  template <typename T>
  requires Shared<T> static T invokeImpl();
  template <typename T>
  requires NonPtr<T> static T invokeImpl();
};

template <typename T, typename R = type_extractor_t<T>>
requires HasInjectCtor<R>&& is_final_v<R> T injectByConstructor() {
  return CtorInvoker<typename R::InjectCtor>::template invoke<T>();
}

template <typename T, typename R = type_extractor_t<T>>
requires(!HasInjectCtor<R>) T injectByConstructor() {
  throwError("Class ", getId<R>(), " is not bound and has no constructors for injection.");
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

// TODO: Copies should be allowed to allow more injection types

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

// Inject a non-ptr finds bindings of:
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
template <typename T>
T CtorInvoker<R(Args...)>::invoke() {
  return invokeImpl<T>();
}

template <typename R, typename... Args>
template <typename T>
requires Unique<T> T CtorInvoker<R(Args...)>::invokeImpl() {
  return make_unique<R>(injector::inject<Args>()...);
}

template <typename R, typename... Args>
template <typename T>
requires Shared<T> T CtorInvoker<R(Args...)>::invokeImpl() {
  return make_shared<R>(injector::inject<Args>()...);
}

template <typename R, typename... Args>
template <typename T>
requires NonPtr<T> T CtorInvoker<R(Args...)>::invokeImpl() {
  return R(injector::inject<Args>()...);
}

}  // namespace

#endif  // INJECTOR_HPP
