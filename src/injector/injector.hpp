#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#include "src/injector/binding_map.hpp"
#include "src/injector/details.hpp"
#include "src/injector/typing.hpp"
#include "src/injector/util.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include <experimental/source_location>

// TODO: Annotations
// ANNOTATED(t1, t2, ..., tn) -> using InjectAnnotations = tuple<t1, t2, ..., tn>
// Annotates first n parameters. Could also make it a static constexpr array so we don't have to
// convert the names during every injection at runtime (this can be applied to the ctor sig as
// well).


#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#define ANNOTATED(...) using InjectAnnotations = std::tuple<__VA_ARGS__>;

namespace injector {

using namespace detail;
using location = std::experimental::source_location;

/**
 * @brief Associates the given type with type Bound.
 * @details Bound must be the same or a base class of To. Injecting a type of Bound will call the
 * corresponding injector for To.
 */
template <typename Bound, typename To, typename Annotation = DefaultAnnotation>
requires Bindable<Bound, To> void bindToClass(const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Bound>(),
      getId<Annotation>(),
      std::any(InjectFunctions<Bound>(
          UniqueSupplier<Bound>(injectImpl<std::unique_ptr<To>, Annotation>),
          SharedSupplier<Bound>(injectImpl<std::shared_ptr<To>, Annotation>),
          NonPtrSupplier<Bound>(injectImpl<To, Annotation>))),
      BindingType::IMPL,
      loc);
}

/**
 * @brief Associates the given object with type Bound.
 *
 * @param obj The object to be bound. Must be of type T, shared_ptr<T>, or unique_ptr<T>, where T is
 * Bound is the same or a base class of T.
 *
 * @note Objects bound via this method will be copied both upon binding and injection.
 */
template <typename Bound, typename ToHolder, typename Annotation = DefaultAnnotation>
requires Bindable<Bound, type_extractor_t<ToHolder>> void bindToObject(
    ToHolder&& obj, const location& loc = location::current()) {
  bindToSupplier<Bound, Annotation>([obj]() { return obj; }, loc);
}

/**
 * @brief Convenience wrapper for bindToObject<Bound, ToHolder>
 * @details Given obj of type T, shared_ptr<T>, or unique_ptr<T>, Bound is automatically inferred as
 * T.
 *
 * @param obj The object to be bound.
 */
template <typename ToHolder, typename Annotation = DefaultAnnotation>
void bindToObject(ToHolder&& obj, const location& loc = location::current()) {
  return bindToObject<type_extractor_t<ToHolder>, ToHolder, Annotation>(
      std::forward<ToHolder>(obj), loc);
}

/**
 * @brief Associates the given supplier with type Bound.
 *
 * @param obj A function that takes in no arguments and returns T, shared_ptr<T>, or unique_ptr<T>,
 * where T is the same as or a base class of Bound.
 */
template <typename Bound, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsUniqueSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Bound>(),
      getId<Annotation>(),
      std::any(UniqueSupplier<Bound>(std::forward<Supplier>(supplier))),
      BindingType::UNIQUE,
      loc);
}

template <typename Bound, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsSharedSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Bound>(),
      getId<Annotation>(),
      std::any(SharedSupplier<Bound>(std::forward<Supplier>(supplier))),
      BindingType::SHARED,
      loc);
}

template <typename Bound, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsNonPtrSupplier<Bound, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Bound>(),
      getId<Annotation>(),
      std::any(NonPtrSupplier<Bound>(std::forward<Supplier>(supplier))),
      BindingType::NON_PTR,
      loc);
}

/**
 * @brief Removes all bindings.
 * @details Mainly for use in unit tests.
 */
void clearBindings() { bindings.clearBindings(); }


/**
 * @brief Retrieves the object associated with type Bound.
 *
 * @return The bound object of type R, given that ToHolder is of type R, unique_ptr<R>, or
 * shared_ptr<R>
 * @throw runtime_error if there is no binding associated with R
 */
template <typename ToHolder, typename Annotation = DefaultAnnotation>
ToHolder inject() {
  try {
    return injectImpl<ToHolder, Annotation>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}

}  // namespace injector

#endif  // INJECTOR_HPP
