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

#include <experimental/source_location>

// TODO: Annotations
// ANNOTATED(t1, t2, ..., tn) -> using InjectAnnotations = tuple<t1, t2, ..., tn>
// Annotates first n parameters. Could also make it a static constexpr array so we don't have to
// convert the names during every injection at runtime (this can be applied to the ctor sig as
// well).

namespace injector {

using namespace detail;
namespace stdexp = std::experimental;

/**********
 * Public *
 **********/

template <typename Bound, typename T>
requires(std::same_as<Bound, T> || std::derived_from<Bound, T>) void bindToInstance(
    std::shared_ptr<T> objPtr,
    const stdexp::source_location& loc = stdexp::source_location::current()) {
  insertBinding(getId<T>(), std::any(std::move(objPtr)), BindingType::INSTANCE, loc);
}

template <typename T>
void bindToInstance(
    std::shared_ptr<T> objPtr,
    const std::experimental::source_location& loc = std::experimental::source_location::current()) {
  return bindToInstance<T, T>(std::move(objPtr), loc);
}

template <typename T, typename Fn>
requires UniqueProvider<T, Fn> void bindToProvider(
    Fn&& provider, const stdexp::source_location& loc = stdexp::source_location::current()) {
  std::function providerFn = std::function<std::unique_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::UNIQUE_PROVIDER, loc);
}

template <typename T, typename Fn>
requires SharedProvider<T, Fn> void bindToProvider(
    Fn&& provider, const stdexp::source_location& loc = stdexp::source_location::current()) {
  std::function providerFn = std::function<std::shared_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::SHARED_PROVIDER, loc);
}

template <typename T, typename Fn>
requires NonPtrProvider<T, Fn> void bindToProvider(
    Fn&& provider, const stdexp::source_location& loc = stdexp::source_location::current()) {
  std::function providerFn = std::function<T(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::NON_PTR_PROVIDER, loc);
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
