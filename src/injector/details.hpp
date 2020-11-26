#ifndef INJECTOR_DETAILS_HPP
#define INJECTOR_DETAILS_HPP

#include "src/injector/binding_map.hpp"
#include "src/injector/inject_exception.hpp"
#include "src/injector/typing.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include <experimental/source_location>

namespace injector {
namespace detail {

  const char CTRL_PATH[] = "Unknown BindingType";

  /*************
   * Injection *
   *************/

  BindingMap bindings;

  template <typename R, typename... Args>
  struct CtorInvoker;

  template <typename R, typename... Args, typename... Annotations>
  struct CtorInvoker<R(Args...), std::tuple<Annotations...>> {
    template <typename T>
    requires Unique<T> static T invoke();
    template <typename T>
    requires Shared<T> static T invoke();
    template <typename T>
    requires NonPtr<T> static T invoke();
  };

  // TODO: Adjust template type names to Bound and ToHolder

  template <typename T, typename R = type_extractor_t<T>>
  requires HasInjectCtor<R>&& std::is_final_v<R> T injectByConstructorImpl() {
    using ExplicitAnnotations = annotation_tuple_t<R>;
    using PaddedAnnotations = tuple_append_n_t<
        ExplicitAnnotations,
        DefaultAnnotation,
        num_args_v<typename R::InjectCtor> - std::tuple_size_v<ExplicitAnnotations>>;
    return CtorInvoker<typename R::InjectCtor, PaddedAnnotations>::template invoke<T>();
  }

  template <typename ToHolder, typename To = type_extractor_t<ToHolder>>
  requires(!HasInjectCtor<To>) ToHolder injectByConstructorImpl() {
    throw InjectException();
  }

  template <typename To, typename ToHolder>  // TODO: Rename Binder -> ToHolder
  ToHolder injectByConstructor() {
    try {
      return injectByConstructorImpl<ToHolder>();
    } catch (InjectException& e) {
      // Build the injection chain for the error message
      e.addClass(getId<To>());
      throw e;
    }
  }

  // Unfortunately, we have to store all of these functions at bind time because we won't know the
  // actual type being supplied at injection time.
  template <typename To>
  struct InjectFunctions {
    InjectFunctions(
        UniqueSupplier<To>&& uniqueInjectFn,
        SharedSupplier<To>&& sharedInjectFn,
        NonPtrSupplier<To>&& nonPtrInjectFn)
        : uniqueInjectFn_(std::move(uniqueInjectFn)),
          sharedInjectFn_(std::move(sharedInjectFn)),
          nonPtrInjectFn_(std::move(nonPtrInjectFn)) {}

    UniqueSupplier<To> uniqueInjectFn_;
    SharedSupplier<To> sharedInjectFn_;
    NonPtrSupplier<To> nonPtrInjectFn_;
  };

  // TODO: any_cast makes a copy unless you refer to the any object by its address.

  template <typename To>
  To extractNonPtr(Binding& binding) {
    return std::any_cast<NonPtrSupplier<To>>(binding.obj)();
  }

  template <typename To>
  std::unique_ptr<To> extractUnique(Binding& binding) {
    return std::any_cast<UniqueSupplier<To>>(binding.obj)();
  }

  template <typename To>
  std::shared_ptr<To> extractShared(Binding& binding) {
    return std::any_cast<SharedSupplier<To>>(binding.obj)();
  }

  template <typename To>
  InjectFunctions<To> extractImpl(Binding& binding) {
    return std::any_cast<InjectFunctions<To>>(binding.obj);
  }

  template <typename Ptr, typename Annotation = DefaultAnnotation>
  requires Unique<Ptr> std::decay_t<Ptr> injectImpl() {
    using T = unique_t<Ptr>;

    Binding* binding = bindings.lookupBinding(getId<T>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<T, std::decay_t<Ptr>>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        return std::make_unique<T>(extractNonPtr<T>(*binding));
      case BindingType::UNIQUE:
        return extractUnique<T>(*binding);
      case BindingType::SHARED:
        return std::make_unique<T>(*extractShared<T>(*binding));
      case BindingType::IMPL:
        try {
          return extractImpl<T>(*binding).uniqueInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<T>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename Ptr, typename Annotation = DefaultAnnotation>
  requires Shared<Ptr> std::decay_t<Ptr> injectImpl() {
    using T = shared_t<Ptr>;

    Binding* binding = bindings.lookupBinding(getId<T>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<T, std::decay_t<Ptr>>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        return std::make_shared<T>(extractNonPtr<T>(*binding));
      case BindingType::UNIQUE:
        // Implicit unique->shared ptr okay
        return extractUnique<T>(*binding);
      case BindingType::SHARED:
        return extractShared<T>(*binding);
      case BindingType::IMPL:
        try {
          return extractImpl<T>(*binding).sharedInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<T>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename T, typename Annotation = DefaultAnnotation>
  requires NonPtr<T> std::decay_t<T> injectImpl() {
    using decT = std::decay_t<T>;

    Binding* binding = bindings.lookupBinding(getId<decT>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<decT, decT>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        return extractNonPtr<decT>(*binding);
      case BindingType::UNIQUE:
        return *extractUnique<decT>(*binding);
      case BindingType::SHARED:
        return *extractShared<decT>(*binding);
      case BindingType::IMPL:
        try {
          return extractImpl<T>(*binding).nonPtrInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<T>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T>
  requires Unique<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invoke() {
    return std::make_unique<R>(injectImpl<Args, Annotations>()...);
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T>
  requires Shared<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invoke() {
    return std::make_shared<R>(injectImpl<Args, Annotations>()...);
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T>
  requires NonPtr<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invoke() {
    return R(injectImpl<Args, Annotations>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
