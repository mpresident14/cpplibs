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
    static T invoke();
    template <typename T, typename PaddedAnnotations>
    requires Unique<T> static T invokeImpl();
    template <typename T, typename PaddedAnnotations>
    requires Shared<T> static T invokeImpl();
    template <typename T, typename PaddedAnnotations>
    requires NonPtr<T> static T invokeImpl();
  };


  template <typename T, typename R = type_extractor_t<T>>
  requires HasInjectCtor<R>&& std::is_final_v<R> T injectByConstructorImpl() {
    return CtorInvoker<typename R::InjectCtor, annotation_tuple_t<R>>::template invoke<T>();
  }

  template <typename T, typename R = type_extractor_t<T>>
  requires(!HasInjectCtor<R>) T injectByConstructorImpl() {
    throw InjectException();
  }

  template <typename T, typename Binder>
  Binder injectByConstructor() {
    try {
      return injectByConstructorImpl<Binder>();
    } catch (InjectException& e) {
      // Build the injection chain for the error message
      e.addClass(getId<T>());
      throw e;
    }
  }

  // Unfortunately, we have to store all of these functions at bind time because we won't know the
  // actual type being supplied at injection time.
  template <typename T>
  struct InjectFunctions {
    InjectFunctions(
        UniqueSupplier<T>&& uniqueInjectFn,
        SharedSupplier<T>&& sharedInjectFn,
        NonPtrSupplier<T>&& nonPtrInjectFn)
        : uniqueInjectFn_(std::move(uniqueInjectFn)),
          sharedInjectFn_(std::move(sharedInjectFn)),
          nonPtrInjectFn_(std::move(nonPtrInjectFn)) {}

    UniqueSupplier<T> uniqueInjectFn_;
    SharedSupplier<T> sharedInjectFn_;
    NonPtrSupplier<T> nonPtrInjectFn_;
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
  requires Unique<Ptr> Ptr injectImpl() {
    using T = unique_t<Ptr>;

    Binding* binding = bindings.lookupBinding(getId<T>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<T, Ptr>();
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
  requires Shared<Ptr> Ptr injectImpl() {
    using T = shared_t<Ptr>;

    Binding* binding = bindings.lookupBinding(getId<T>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<T, Ptr>();
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
  requires NonPtr<T> T injectImpl() {
    using decT = std::decay_t<T>;

    Binding* binding = bindings.lookupBinding(getId<T>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<T, T>();
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

  // TODO: Add AnnotationTuple to template and figure out how to get each one into injectImpl

  template <typename R, typename... Args, typename... Annotations>
  template <typename T>
  T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invoke() {
    using PaddedAnnotations = tuple_append_n_t<
        std::tuple<Annotations...>,
        DefaultAnnotation,
        sizeof...(Args) - sizeof...(Annotations)>;
    return invokeImpl<T, PaddedAnnotations>();
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T, typename PaddedAnnotations>
  requires Unique<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invokeImpl() {
    return std::make_unique<R>(injectImpl<Args, PaddedAnnotations>()...);
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T, typename PaddedAnnotations>
  requires Shared<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invokeImpl() {
    return std::make_shared<R>(injectImpl<Args, PaddedAnnotations>()...);
  }

  template <typename R, typename... Args, typename... Annotations>
  template <typename T, typename PaddedAnnotations>
  requires NonPtr<T> T CtorInvoker<R(Args...), std::tuple<Annotations...>>::invokeImpl() {
    return R(injectImpl<Args, PaddedAnnotations>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
