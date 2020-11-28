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
 * @brief Associates type Value with type Key, such that injecting a Key will call the corresponding
 * injector for Value.
 *
 * @param <Key> The type to be bound. Must be decayed and the same or convertible to Value.
 * @param <Value> The class to which Key is bound.
 * @param <Annotation> An extra identifier to differentiate between multiple bindings of the same
 * Key.
 *
 * @throw runtime_error if there is already a binding associated with Key
 */
template <typename Key, typename Value, typename Annotation = Unannotated>
requires ImplBindable<Key, Value> void bindToBase(const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(InjectFunctions<Key>(
          UniqueSupplier<Key>(injectImpl<std::unique_ptr<Value>, Annotation>),
          SharedSupplier<Key>(injectImpl<std::shared_ptr<Value>, Annotation>))),
      false,
      BindingType::IMPL,
      loc);
}

/**
 * @brief Associates the given value with type Key,
 *
 * @param <Key> The type to be bound. Must be a non-volatile non-reference.
 * @param <Annotation> An extra identifier to differentiate between multiple bindings of the same
 * @param val The value to which Key will be bound. Must be of type Value, shared_ptr<Value>, or
 * unique_ptr<Value>, where Value is convertible to Key
 *
 * @throw runtime_error if there is already a binding associated with Key
 *
 * @note Objects bound via this method are copied upon injection.
 */
template <typename Key, typename Annotation = Unannotated, typename ValueHolder>
requires ValidKey<Key> void bindToObject(
    ValueHolder&& val, const location& loc = location::current()) {
  bindToSupplier<Key, Annotation>([val = std::forward<ValueHolder>(val)]() { return val; }, loc);
}

/**
 * @brief Convenience wrapper for bindToObject<Key, Annotation, ValueHolder> where Key is
 * automatically inferred.
 *
 * @param val The value to which Key will be bound. Given val of type Value, shared_ptr<Value>, or
 * unique_ptr<Value>, Key is automatically inferred as Value. Key must be able to be inferred as a
 * non-volatile non-reference.
 */
template <typename Annotation = Unannotated, typename ValueHolder>
void bindToObject(ValueHolder&& val, const location& loc = location::current()) {
  return bindToObject<type_extractor_t<ValueHolder>, Annotation, ValueHolder>(
      std::forward<ValueHolder>(val), loc);
}

/**
 * @brief Associates the given supplier with type Key.
 *
 * @param <Key> The type to be bound. Must be a non-volatile non-reference.
 * @param supplier A function that takes in no arguments and returns Value, shared_ptr<Value>, or
 * unique_ptr<Value>, where Value is convertible Key. The return type of supplier must be
 * move-constructible.
 *
 * @throw runtime_error if there is already a binding associated with Key
 */
template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsUniqueSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(UniqueSupplier<Key>(std::forward<Supplier>(supplier))),
      std::is_const_v<Key>,
      BindingType::UNIQUE,
      loc);
}

template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsSharedSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(SharedSupplier<Key>(std::forward<Supplier>(supplier))),
      std::is_const_v<Key>,
      BindingType::SHARED,
      loc);
}

template <typename Key, typename Annotation = Unannotated, typename Supplier>
requires ValidKey<Key>&& IsNonPtrSupplier<Key, Supplier> void bindToSupplier(
    Supplier&& supplier, const location& loc = location::current()) {
  bindings.insertBinding<Annotation>(
      getId<Key>(),
      std::any(NonPtrSupplier<Key>(std::forward<Supplier>(supplier))),
      false,
      BindingType::NON_PTR,
      loc);
}

/**
 * @brief Removes all bindings.
 * @details Mainly for use in unit tests.
 */
void clearBindings() { bindings.clearBindings(); }


/**
 * @brief Retrieves the value of a binding.
 *
 * @param KeyHolder The type of the object to inject.  Given KeyHolder of type Key, shared_ptr<Key>,
 * or unique_ptr<Key>, Key must be a non-volatile non-reference.
 *
 * @throw runtime_error if there is no binding associated with Key
 */
template <typename KeyHolder, typename Annotation = Unannotated>
requires ValidKey<type_extractor_t<KeyHolder>> KeyHolder inject() {
  try {
    return injectImpl<std::decay_t<KeyHolder>, Annotation>();
  } catch (InjectException& e) {
    std::ostringstream err;
    err << e;
    throw std::runtime_error(err.str());
  }
}


class BindingModule {
public:
  virtual void install() = 0;
};

}  // namespace injector

#endif  // INJECTOR_HPP
