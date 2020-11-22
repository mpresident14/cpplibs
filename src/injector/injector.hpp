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

/**
 * @brief Associates the given type with type Bound.
 * @details Bound must be the same or a base class of To. Injecting a type of Bound will call the
 * corresponding injector for To.
 */
template <typename Bound, typename To>
requires Bindable<Bound, To> void bindToClass(const location& loc = location::current()) {
  bindToSupplier<Bound>([]() { return inject<To>(); });
}

/**
 * @brief Associates the given object with type Bound.
 *
 * @param obj The object to be bound. Must be of type T, shared_ptr<T>, or unique_ptr<T>, where T is
 * Bound is the same or a base class of T.
 *
 * @note Objects bound via this method will be copied both upon binding and injection.
 */
template <typename Bound, typename ToHolder>
requires Bindable<Bound, type_extractor_t<ToHolder>> void bindToObject(
    ToHolder&& obj, const location& loc = location::current()) {
  bindToSupplier<Bound>([obj]() { return obj; }, loc);
}

/**
 * @brief Convenience wrapper for bindToObject<Bound, ToHolder>
 * @details Given obj of type T, shared_ptr<T>, or unique_ptr<T>, Bound is automatically inferred as
 * T.
 *
 * @param obj The object to be bound.
 */
template <typename ToHolder>
void bindToObject(ToHolder&& obj, const location& loc = location::current()) {
  return bindToObject<type_extractor_t<ToHolder>, ToHolder>(std::forward<ToHolder>(obj), loc);
}

/**
 * @brief Associates the given supplier with type Bound.
 *
 * @param obj A function that takes in no arguments and returns T, shared_ptr<T>, or unique_ptr<T>,
 * where T is Bound is the same or a base class of T.
 */
template <typename Bound, typename Supplier>
requires UniqueSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  std::function supplierFn =
      std::function<std::unique_ptr<Bound>(void)>(std::forward<Supplier>(supplier));
  insertBinding(getId<Bound>(), std::any(std::move(supplierFn)), BindingType::UNIQUE, loc);
}

template <typename Bound, typename Supplier>
requires SharedSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  std::function supplierFn =
      std::function<std::shared_ptr<Bound>(void)>(std::forward<Supplier>(supplier));
  insertBinding(getId<Bound>(), std::any(std::move(supplierFn)), BindingType::SHARED, loc);
}

template <typename Bound, typename Supplier>
requires NonPtrSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  std::function supplierFn = std::function<Bound(void)>(std::forward<Supplier>(supplier));
  insertBinding(getId<Bound>(), std::any(std::move(supplierFn)), BindingType::NON_PTR, loc);
}

/**
 * @brief Retrieves the object associated with type Bound.
 *
 * @return The bound object of type R, given that ToHolder is of type R, unique_ptr<R>, or
 * shared_ptr<R>
 * @throw runtime_error if there is no binding associated with R
 */
template <typename ToHolder>
ToHolder inject() {
  try {
    return injectImpl<ToHolder>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}


void clearBindings() { bindings.clear(); }

}  // namespace injector

#endif  // INJECTOR_HPP
