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
  const char UNIQUE_PTR[] = "unique_ptr";
  const char SHARED_PTR[] = "shared_ptr";
  const char OBJECT[] = "object";

  /*************
   * Injection *
   *************/

  BindingMap bindings;

  template <typename Value, typename... Args>
  struct CtorInvoker;

  template <typename Value, typename... Args, typename... Annotations>
  struct CtorInvoker<Value(Args...), std::tuple<Annotations...>> {
    template <typename ValueHolder>
    requires Unique<ValueHolder> static ValueHolder invoke();
    template <typename ValueHolder>
    requires Shared<ValueHolder> static ValueHolder invoke();
    template <typename ValueHolder>
    requires NonPtr<ValueHolder> static ValueHolder invoke();
  };


  // TODO: Shouldn't have to instantiate a different version of this for each Annotation
  template <
      typename ValueHolder,
      typename Annotation,
      typename Value = value_extractor_t<ValueHolder>>
  requires HasInjectCtor<Value>&& std::is_final_v<Value> ValueHolder injectByConstructorImpl() {
    using ExplicitAnnotations = annotation_tuple_t<Value>;
    using PaddedAnnotations = tuple_append_n_t<
        ExplicitAnnotations,
        DefaultAnnotation,
        num_args_v<typename Value::InjectCtor> - std::tuple_size_v<ExplicitAnnotations>>;
    return CtorInvoker<typename Value::InjectCtor, PaddedAnnotations>::template invoke<
        ValueHolder>();
  }

  template <
      typename ValueHolder,
      typename Annotation,
      typename Value = value_extractor_t<ValueHolder>>
  requires(!HasInjectCtor<Value>) ValueHolder injectByConstructorImpl() {
    std::ostringstream err;
    err << "Type " << getId<Value>();
    streamNonDefault(err, getId<Annotation>());
    err << " is not bound and has no constructors for injection.";
    throw InjectException(err.str());
  }

  template <typename Value, typename ValueHolder, typename Annotation>
  ValueHolder injectByConstructor() {
    try {
      return injectByConstructorImpl<ValueHolder, Annotation>();
    } catch (InjectException& e) {
      // Build the injection chain for the error message
      e.addClass(getId<Value>(), getId<Annotation>());
      throw e;
    }
  }

  // Unfortunately, we have to store all of these functions at bind time because we won't know the
  // actual type being supplied at injection time.
  template <typename Value>
  struct InjectFunctions {
    InjectFunctions(
        UniqueSupplier<Value>&& uniqueInjectFn,
        SharedSupplier<Value>&& sharedInjectFn,
        NonPtrSupplier<Value>&& nonPtrHolderInjectFn)
        : uniqueInjectFn_(std::move(uniqueInjectFn)),
          sharedInjectFn_(std::move(sharedInjectFn)),
          nonPtrHolderInjectFn_(std::move(nonPtrHolderInjectFn)) {}

    UniqueSupplier<Value> uniqueInjectFn_;
    SharedSupplier<Value> sharedInjectFn_;
    NonPtrSupplier<Value> nonPtrHolderInjectFn_;
  };

  // TODO: In accordance with the "no copies" rule, these methods should throw InjectionException
  // with wrong binding type message
  template <typename Value>
  Value extractNonPtr(Binding& binding) {
    return (*std::any_cast<NonPtrSupplier<Value>>(&binding.obj))();
  }

  template <typename Value>
  std::unique_ptr<Value> extractUnique(Binding& binding) {
    return (*std::any_cast<UniqueSupplier<Value>>(&binding.obj))();
  }

  template <typename Value>
  std::shared_ptr<Value> extractShared(Binding& binding) {
    return (*std::any_cast<SharedSupplier<Value>>(&binding.obj))();
  }

  template <typename Value>
  InjectFunctions<Value>* extractImpl(Binding& binding) {
    return std::any_cast<InjectFunctions<Value>>(&binding.obj);
  }


  template <typename Value, typename Annotation>
  void wrongBindingError(const char* bound, const char* injected) {
    InjectException e(strCat(
        "Incompatible binding for type ",
        getId<Value>(),
        ". Cannot not inject ",
        injected,
        " from a ",
        bound,
        " binding."));
    e.addClass(getId<Value>(), getId<Annotation>());
    throw e;
  }

  // TODO: Prevent all copies from being made because classes w/o copy ctor fail to compile when
  // trying to instantiate these functions. Delete copy constructors in all test classes.

  template <typename ValueHolder, typename Annotation = DefaultAnnotation>
  requires Unique<ValueHolder> std::decay_t<ValueHolder> injectImpl() {
    using Value = unique_t<ValueHolder>;

    Binding* binding = bindings.lookupBinding(getId<Value>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<Value, std::decay_t<ValueHolder>, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        // TODO: Remove copies
        // return std::make_unique<Value>(extractNonPtr<Value>(*binding));
        wrongBindingError<Value, Annotation>(OBJECT, UNIQUE_PTR);
      case BindingType::UNIQUE:
        return extractUnique<Value>(*binding);
      case BindingType::SHARED:
        // TODO: Remove copies
        // return std::make_unique<Value>(*extractShared<Value>(*binding));
        wrongBindingError<Value, Annotation>(SHARED_PTR, UNIQUE_PTR);
      case BindingType::IMPL:
        try {
          return extractImpl<Value>(*binding)->uniqueInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<Value>(), getId<Annotation>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename ValueHolder, typename Annotation = DefaultAnnotation>
  requires Shared<ValueHolder> std::decay_t<ValueHolder> injectImpl() {
    using Value = shared_t<ValueHolder>;

    Binding* binding = bindings.lookupBinding(getId<Value>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<Value, std::decay_t<ValueHolder>, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        // TODO: Remove copies
        // return std::make_shared<Value>(extractNonPtr<Value>(*binding));
        wrongBindingError<Value, Annotation>(OBJECT, SHARED_PTR);
      case BindingType::UNIQUE:
        // Implicit unique->shared ptr okay
        return extractUnique<Value>(*binding);
      case BindingType::SHARED:
        return extractShared<Value>(*binding);
      case BindingType::IMPL:
        try {
          return extractImpl<Value>(*binding)->sharedInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<Value>(), getId<Annotation>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename ValueHolder, typename Annotation = DefaultAnnotation>
  requires NonPtr<ValueHolder> std::decay_t<ValueHolder> injectImpl() {
    using Value = std::decay_t<ValueHolder>;

    Binding* binding = bindings.lookupBinding(getId<Value>(), getId<Annotation>());
    if (!binding) {
      return injectByConstructor<Value, Value, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        return extractNonPtr<Value>(*binding);
      case BindingType::UNIQUE:
        // TODO: Remove copies
        // return *extractUnique<Value>(*binding);
        wrongBindingError<Value, Annotation>(UNIQUE_PTR, OBJECT);
      case BindingType::SHARED:
        // TODO: Remove copies
        // return *extractShared<Value>(*binding);
        wrongBindingError<Value, Annotation>(SHARED_PTR, OBJECT);
      case BindingType::IMPL:
        try {
          return extractImpl<Value>(*binding)->nonPtrHolderInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<Value>(), getId<Annotation>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename Value, typename... Args, typename... Annotations>
  template <typename ValueHolder>
  requires Unique<ValueHolder> ValueHolder
  CtorInvoker<Value(Args...), std::tuple<Annotations...>>::invoke() {
    return std::make_unique<Value>(injectImpl<Args, Annotations>()...);
  }

  template <typename Value, typename... Args, typename... Annotations>
  template <typename ValueHolder>
  requires Shared<ValueHolder> ValueHolder
  CtorInvoker<Value(Args...), std::tuple<Annotations...>>::invoke() {
    return std::make_shared<Value>(injectImpl<Args, Annotations>()...);
  }

  template <typename Value, typename... Args, typename... Annotations>
  template <typename ValueHolder>
  requires NonPtr<ValueHolder> ValueHolder
  CtorInvoker<Value(Args...), std::tuple<Annotations...>>::invoke() {
    return Value(injectImpl<Args, Annotations>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
