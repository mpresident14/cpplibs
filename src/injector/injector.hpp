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


#define INJECT(ctorDecl)       \
  using InjectCtor = ctorDecl; \
  ctorDecl

#define ANNOTATED(...) using InjectAnnotations = std::tuple<__VA_ARGS__>;

namespace injector {

using namespace detail;
using location = std::experimental::source_location;

/**
 * @brief Associates the given type with type Key.
 * @details Key must be the same or a base class of Value. Injecting a type of Key will call the
 * corresponding injector for Value.
 */
template <typename Key, typename Value, typename Annotation = DefaultAnnotation>
requires Bindable<Key, Value> void bindToClass(const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Key>(),
      getId<Annotation>(),
      std::any(InjectFunctions<Key>(
          UniqueSupplier<Key>(injectImpl<std::unique_ptr<Value>, Annotation>),
          SharedSupplier<Key>(injectImpl<std::shared_ptr<Value>, Annotation>),
          NonPtrSupplier<Key>(injectImpl<Value, Annotation>))),
      BindingType::IMPL,
      loc);
}

/**
 * @brief Associates the given object with type Key.
 *
 * @param obj The object to be bound. Must be of type T, shared_ptr<T>, or unique_ptr<T>, where T is
 * Key is the same or a base class of T.
 *
 * @note Objects bound via this method will be copied upon binding.
 */
template <typename Key, typename Annotation = DefaultAnnotation, typename ValueHolder>
requires Bindable<Key, value_extractor_t<ValueHolder>> void bindToObject(
    ValueHolder&& obj, const location& loc = location::current()) {
  bindToSupplier<Key, Annotation>([obj = std::forward<ValueHolder>(obj)]() { return obj; }, loc);
}

/**
 * @brief Convenience wrapper for bindToObject<Key, ValueHolder>
 * @details Given obj of type T, shared_ptr<T>, or unique_ptr<T>, Key is automatically inferred as
 * T.
 *
 * @param obj The object to be bound.
 */
template <typename Annotation = DefaultAnnotation, typename ValueHolder>
void bindToObject(ValueHolder&& obj, const location& loc = location::current()) {
  return bindToObject<value_extractor_t<ValueHolder>, Annotation, ValueHolder>(
      std::forward<ValueHolder>(obj), loc);
}

/**
 * @brief Associates the given supplier with type Key.
 *
 * @param obj A function that takes in no arguments and returns T, shared_ptr<T>, or unique_ptr<T>,
 * where T is the same as or a base class of Key.
 */
template <typename Key, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsUniqueSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Key>(),
      getId<Annotation>(),
      std::any(UniqueSupplier<Key>(std::forward<Supplier>(supplier))),
      BindingType::UNIQUE,
      loc);
}

template <typename Key, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsSharedSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Key>(),
      getId<Annotation>(),
      std::any(SharedSupplier<Key>(std::forward<Supplier>(supplier))),
      BindingType::SHARED,
      loc);
}

template <typename Key, typename Annotation = DefaultAnnotation, typename Supplier>
requires IsNonPtrSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding(
      getId<Key>(),
      getId<Annotation>(),
      std::any(NonPtrSupplier<Key>(std::forward<Supplier>(supplier))),
      BindingType::NON_PTR,
      loc);
}

/**
 * @brief Removes all bindings.
 * @details Mainly for use in unit tests.
 */
void clearBindings() { bindings.clearBindings(); }


/**
 * @brief Retrieves the object associated with type Key.
 *
 * @return The bound object of type R, given that ValueHolder is of type R, unique_ptr<R>, or
 * shared_ptr<R>
 * @throw runtime_error if there is no binding associated with R
 */
template <typename ValueHolder, typename Annotation = DefaultAnnotation>
ValueHolder inject() {
  try {
    return injectImpl<std::decay_t<ValueHolder>, Annotation>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}

}  // namespace injector

#endif  // INJECTOR_HPP
