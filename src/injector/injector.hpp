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
using location = std::experimental::source_location;

/**********
 * Public *
 **********/

template <typename Bound, typename T>
requires(std::same_as<Bound, T> || std::derived_from<Bound, T>) void bindToInstance(
    std::shared_ptr<T> objPtr, const location& loc = location::current()) {
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
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<std::unique_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::UNIQUE_PROVIDER, loc);
}

template <typename T, typename Fn>
requires SharedProvider<T, Fn> void bindToProvider(
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<std::shared_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::SHARED_PROVIDER, loc);
}

template <typename T, typename Fn>
requires NonPtrProvider<T, Fn> void bindToProvider(
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<T(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::NON_PTR_PROVIDER, loc);
}


template <typename T>
T inject() {
  try {
    return injectImpl<T>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}


void clearBindings() { bindings.clear(); }

}  // namespace injector

#endif  // INJECTOR_HPP
