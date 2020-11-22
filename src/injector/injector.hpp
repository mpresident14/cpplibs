#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#include "src/injector/details.hpp"
#include "src/injector/typing.hpp"

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
      return std::make_unique<T>(extractNonPtr<T>(binding));
    case BindingType::UNIQUE_PROVIDER:
      return extractUnique<T>(binding);
    case BindingType::SHARED_PROVIDER:
      return std::make_unique<T>(*extractShared<T>(binding));
    case BindingType::INSTANCE:
      return std::make_unique<T>(*extractInstance<T>(binding));
  }

  throw std::runtime_error(CTRL_PATH);
}

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
      return std::make_shared<T>(extractNonPtr<T>(binding));
    case BindingType::UNIQUE_PROVIDER:
      // Implicit unique->shared ptr okay
      return extractUnique<T>(binding);
    case BindingType::SHARED_PROVIDER:
      return extractShared<T>(binding);
    case BindingType::INSTANCE:
      return extractInstance<T>(binding);
  }

  throw std::runtime_error(CTRL_PATH);
}

// TODO: Decay removes references, but NonPtr provider may actually produce a reference

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
      return extractNonPtr<decT>(binding);
    case BindingType::UNIQUE_PROVIDER:
      return std::move(*extractUnique<decT>(binding));
    case BindingType::SHARED_PROVIDER:
      return std::move(*extractShared<decT>(binding));
    case BindingType::INSTANCE:
      return *extractInstance<decT>(binding);
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
