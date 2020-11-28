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

  static const char CTRL_PATH[] = "Unknown BindingType";
  static const char UNIQUE_PTR[] = "unique_ptr";
  static const char SHARED_PTR[] = "shared_ptr";
  static const char NON_PTR[] = "non-pointer";

  /*************
   * Injection *
   *************/

  BindingMap bindings;

  template <typename Key, typename... ArgsCVRef>
  struct CtorInvoker;

  template <typename Key, typename... ArgsCVRef, typename... Annotations>
  struct CtorInvoker<Key(ArgsCVRef...), std::tuple<Annotations...>> {
    template <typename KeyHolder>
    requires Unique<KeyHolder> static KeyHolder invoke();
    template <typename KeyHolder>
    requires Shared<KeyHolder> static KeyHolder invoke();
    template <typename KeyHolder>
    requires NonPtr<KeyHolder> static KeyHolder invoke();
  };


  template <typename KeyHolder, typename Annotation, typename Key = value_extractor_t<KeyHolder>>
  requires HasInjectCtor<Key>&& std::is_final_v<Key> KeyHolder injectByConstructorImpl() {
    using ExplicitAnnotations = annotation_tuple_t<Key>;
    using PaddedAnnotations = tuple_append_n_t<
        ExplicitAnnotations,
        Unannotated,
        num_args_v<typename Key::InjectCtor> - std::tuple_size_v<ExplicitAnnotations>>;
    return CtorInvoker<typename Key::InjectCtor, PaddedAnnotations>::template invoke<KeyHolder>();
  }

  template <typename KeyHolder, typename Annotation, typename Key = value_extractor_t<KeyHolder>>
  requires(!HasInjectCtor<Key>) KeyHolder injectByConstructorImpl() {
    std::ostringstream err;
    err << "Type " << getId<Key>();
    streamAnnotated<Annotation>(err);
    err << " is not bound and has no constructors for injection.";
    throw InjectException(err.str());
  }

  template <typename Key, typename KeyHolder, typename Annotation>
  KeyHolder injectByConstructor() {
    try {
      return injectByConstructorImpl<KeyHolder, Annotation>();
    } catch (InjectException& e) {
      // Build the injection chain for the error message
      e.addClass(getId<Key>(), getId<Annotation>());
      throw e;
    }
  }

  template <typename Key>
  struct InjectFunctions {
    InjectFunctions(UniqueSupplier<Key>&& uniqueInjectFn, SharedSupplier<Key>&& sharedInjectFn)
        : uniqueInjectFn_(std::move(uniqueInjectFn)), sharedInjectFn_(std::move(sharedInjectFn)) {}

    UniqueSupplier<Key> uniqueInjectFn_;
    SharedSupplier<Key> sharedInjectFn_;
  };

  template <typename Key>
  Key extractNonPtr(Binding& binding) {
    return (*std::any_cast<NonPtrSupplier<Key>>(&binding.obj))();
  }

  template <typename Key>
  std::unique_ptr<Key> extractUnique(Binding& binding) {
    return (*std::any_cast<UniqueSupplier<Key>>(&binding.obj))();
  }

  template <typename Key>
  std::shared_ptr<Key> extractShared(Binding& binding) {
    return (*std::any_cast<SharedSupplier<Key>>(&binding.obj))();
  }

  template <typename Key>
  InjectFunctions<Key>* extractImpl(Binding& binding) {
    return std::any_cast<InjectFunctions<Key>>(&binding.obj);
  }


  template <typename Key, typename Annotation>
  void wrongBindingError(const char* bound, const char* injected) {
    InjectException e(strCat(
        "Incompatible binding for type ",
        getId<Key>(),
        ". Cannot not inject ",
        injected,
        " from a ",
        bound,
        " binding."));
    e.addClass(getId<Key>(), getId<Annotation>());
    throw e;
  }

  template <typename KeyHolder, typename Annotation = Unannotated>
  requires Unique<KeyHolder> KeyHolder injectImpl() {
    using Key = unique_t<KeyHolder>;

    Binding* binding = bindings.lookupBinding<Annotation>(getId<Key>());
    if (!binding) {
      return injectByConstructor<Key, KeyHolder, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        wrongBindingError<Key, Annotation>(NON_PTR, UNIQUE_PTR);
      case BindingType::UNIQUE:
        return extractUnique<Key>(*binding);
      case BindingType::SHARED:
        wrongBindingError<Key, Annotation>(SHARED_PTR, UNIQUE_PTR);
      case BindingType::IMPL:
        try {
          return extractImpl<Key>(*binding)->uniqueInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<Key>(), getId<Annotation>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename KeyHolder, typename Annotation = Unannotated>
  requires Shared<KeyHolder> KeyHolder injectImpl() {
    using Key = shared_t<KeyHolder>;

    Binding* binding = bindings.lookupBinding<Annotation>(getId<Key>());
    if (!binding) {
      return injectByConstructor<Key, KeyHolder, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        wrongBindingError<Key, Annotation>(NON_PTR, SHARED_PTR);
      case BindingType::UNIQUE:
        // Implicit unique->shared ptr okay
        return extractUnique<Key>(*binding);
      case BindingType::SHARED:
        return extractShared<Key>(*binding);
      case BindingType::IMPL:
        try {
          return extractImpl<Key>(*binding)->sharedInjectFn_();
        } catch (InjectException& e) {
          e.addClass(getId<Key>(), getId<Annotation>());
          throw e;
        }
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename KeyHolder, typename Annotation = Unannotated>
  requires NonPtr<KeyHolder> KeyHolder injectImpl() {
    using Key = KeyHolder;

    Binding* binding = bindings.lookupBinding<Annotation>(getId<Key>());
    if (!binding) {
      return injectByConstructor<Key, Key, Annotation>();
    }

    switch (binding->type) {
      case BindingType::NON_PTR:
        return extractNonPtr<Key>(*binding);
      case BindingType::UNIQUE:
        wrongBindingError<Key, Annotation>(UNIQUE_PTR, NON_PTR);
      case BindingType::SHARED:
        wrongBindingError<Key, Annotation>(SHARED_PTR, NON_PTR);
      case BindingType::IMPL:
        wrongBindingError<Key, Annotation>("base class", NON_PTR);
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename Key, typename... ArgsCVRef, typename... Annotations>
  template <typename KeyHolder>
  requires Unique<KeyHolder> KeyHolder
  CtorInvoker<Key(ArgsCVRef...), std::tuple<Annotations...>>::invoke() {
    return std::make_unique<Key>(injectImpl<std::decay_t<ArgsCVRef>, Annotations>()...);
  }

  template <typename Key, typename... ArgsCVRef, typename... Annotations>
  template <typename KeyHolder>
  requires Shared<KeyHolder> KeyHolder
  CtorInvoker<Key(ArgsCVRef...), std::tuple<Annotations...>>::invoke() {
    return std::make_shared<Key>(injectImpl<std::decay_t<ArgsCVRef>, Annotations>()...);
  }

  template <typename Key, typename... ArgsCVRef, typename... Annotations>
  template <typename KeyHolder>
  requires NonPtr<KeyHolder> KeyHolder
  CtorInvoker<Key(ArgsCVRef...), std::tuple<Annotations...>>::invoke() {
    return Key(injectImpl<std::decay_t<ArgsCVRef>, Annotations>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
