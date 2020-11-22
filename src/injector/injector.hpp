#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#include "src/injector/typing.hpp"
#include "src/injector/details.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace injector {

using namespace detail;

/**********
 * Public *
 **********/

template <typename Bound, typename T>
    requires std::same_as<Bound, T> || std::derived_from<Bound, T> void bindToInstance(std::shared_ptr<T> objPtr) {
  if (!bindings.emplace(getId<T>(), Binding{ BindingType::INSTANCE, std::any(std::move(objPtr)) })
           .second) {
    // TODO: Use source::location  in public methods so that we can output line and file of original
    // binding
    throwError("Binding for type ", getId<T>(), " already exists.");
  }
}

template <typename T>
void bindToInstance(std::shared_ptr<T> objPtr) {
  return bindToInstance<T, T>(std::move(objPtr));
}

// TODO: concept for provider for better error msgs
template <typename T, typename Fn>
requires UniqueProvider<T, Fn> void bindToProvider(Fn&& provider) {
  std::function providerFn = std::function<std::unique_ptr<T>(void)>(std::forward<Fn>(provider));
  return bindToProviderImpl<T>(std::move(providerFn), BindingType::UNIQUE_PROVIDER);
}

template <typename T, typename Fn>
requires SharedProvider<T, Fn> void bindToProvider(Fn&& provider) {
  std::function providerFn = std::function<std::shared_ptr<T>(void)>(std::forward<Fn>(provider));
  return bindToProviderImpl<T>(std::move(providerFn), BindingType::SHARED_PROVIDER);
}

template <typename T, typename Fn>
requires NonPtrProvider<T, Fn> void bindToProvider(Fn&& provider) {
  std::function providerFn = std::function<T(void)>(std::forward<Fn>(provider));
  return bindToProviderImpl<T>(std::move(providerFn), BindingType::NON_PTR_PROVIDER);
}

// TODO: Wrap std::any_cast exceptions with helpful errors
// TODO: Can we consolidate these cases into a single std::function (or at least share the main
// std::functionality)?
// TODO: Copies should be allowed to allow more injection types

// Inject a unique ptr finds bindings of:
// -non-ptr providers
// +unique ptr providers
// -std::shared_ptr providers
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
      wrongBindingError<T>("std::unique_ptr", "object");
    case BindingType::UNIQUE_PROVIDER:
      return std::any_cast<std::function<Ptr(void)>>(binding.obj)();
    case BindingType::SHARED_PROVIDER:
    case BindingType::INSTANCE:
      wrongBindingError<T>("std::unique_ptr ", "std::shared_ptr");
  }

  throw std::runtime_error(CTRL_PATH);
}

// Inject a shared ptr finds bindings of:
// -non-ptr providers
// +unique ptr providers
// +std::shared_ptr providers
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
      wrongBindingError<T>("std::shared_ptr", "object");
    case BindingType::UNIQUE_PROVIDER:
      // Implicit unique->shared ptr okay
      return std::any_cast<std::function<std::unique_ptr<T>(void)>>(binding.obj)();
    case BindingType::SHARED_PROVIDER:
      return std::any_cast<std::function<std::shared_ptr<T>(void)>>(binding.obj)();
    case BindingType::INSTANCE:
      return std::any_cast<std::shared_ptr<T>>(binding.obj);
  }

  throw std::runtime_error(CTRL_PATH);
}

// TODO: Use decltype(auto) to get const references eliminate these std::moves ???

// Inject a non-ptr finds bindings of:
// +non-ptr providers
// +unique ptr providers
// +std::shared_ptr providers
// +instances
template <typename T>
requires NonPtr<T> T inject() {
  using decT = std::decay_t<T>;
  auto iter = bindings.find(getId<T>());
  if (iter == bindings.end()) {
    return injectByConstructor<T>();
  }

  Binding& binding = iter->second;
  switch (binding.type) {
    case BindingType::NON_PTR_PROVIDER:
      return std::any_cast<std::function<decT(void)>>(binding.obj)();
    case BindingType::UNIQUE_PROVIDER:
      return std::move(*std::any_cast<std::function<std::unique_ptr<decT>(void)>>(binding.obj)());
    case BindingType::SHARED_PROVIDER:
      return std::move(*std::any_cast<std::function<std::shared_ptr<decT>(void)>>(binding.obj)());
    case BindingType::INSTANCE:
      return *std::any_cast<std::shared_ptr<decT>>(binding.obj);
  }

  throw std::runtime_error(CTRL_PATH);
}


void clearBindings() { bindings.clear(); }


namespace detail {

  template <typename R, typename... Args>
  template <typename T>
  T CtorInvoker<R(Args...)>::invoke() {
    return invokeImpl<T>();
  }

  template <typename R, typename... Args>
  template <typename T>
  requires Unique<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return std::make_unique<R>(injector::inject<Args>()...);
  }

  template <typename R, typename... Args>
  template <typename T>
  requires Shared<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return std::make_shared<R>(injector::inject<Args>()...);
  }

  template <typename R, typename... Args>
  template <typename T>
  requires NonPtr<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return R(injector::inject<Args>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_HPP
