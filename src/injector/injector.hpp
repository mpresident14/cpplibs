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


// Bind
// - IF to impl
// - Type to instance
// - Type to some provider


// Want to inject
// - shared_ptr (singleton)
// - unique_ptr (new obj)
// - copy (primitive/cheap copy)

namespace injector {

using namespace detail;
using location = std::experimental::source_location;

/**********
 * Public *
 **********/

/*
 * For primitives and cheaply copied objects (e.g. shared_ptrs)
 * - lambda captures by copy and returns a copy of the captured value on injection
 */
template <typename Bound, typename ToHolder>
requires(std::same_as<type_extractor_t<ToHolder>, Bound> || std::derived_from<type_extractor_t<ToHolder>, Bound>) void bindToInstance(
    ToHolder&& obj, const location& loc = location::current()) {
  bindToProvider<Bound>([obj]() { return obj; }, loc);
}

/* Convenience method for binding a value to its own type */
template <typename ToHolder>
void bindToInstance(ToHolder&& obj, const location& loc = location::current()) {
  return bindToInstance<type_extractor_t<ToHolder>, ToHolder>(std::forward<ToHolder>(obj), loc);
}

template <typename T, typename Fn>
requires UniqueProvider<T, Fn> void bindToProvider(
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<std::unique_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::UNIQUE, loc);
}

template <typename T, typename Fn>
requires SharedProvider<T, Fn> void bindToProvider(
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<std::shared_ptr<T>(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::SHARED, loc);
}

template <typename T, typename Fn>
requires NonPtrProvider<T, Fn> void bindToProvider(
    Fn&& provider, const location& loc = location::current()) {
  std::function providerFn = std::function<T(void)>(std::forward<Fn>(provider));
  insertBinding(getId<T>(), std::any(std::move(providerFn)), BindingType::NON_PTR, loc);
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
