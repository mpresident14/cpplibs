#ifndef INJECTOR_DETAILS_HPP
#define INJECTOR_DETAILS_HPP

#include "src/injector/inject_exception.hpp"
#include "src/injector/typing.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <experimental/source_location>

namespace injector {

namespace detail {

  const char CTRL_PATH[] = "Unknown BindingType";


  /**************
   * Containers *
   **************/
  enum class BindingType { UNIQUE, SHARED, NON_PTR, IMPL };
  struct Binding {
    BindingType type;
    std::any obj;
    const char* filename;
    size_t line;
  };

  /*************
   * Variables *
   *************/
  std::unordered_map<std::string, Binding> bindings;


  /******************
   * Util Functions *
   ******************/

  template <typename T>
  constexpr const char* getId() {
    return typeid(std::decay_t<T>).name();
  }

  template <typename... Args>
  void throwError(Args... msgParts) {
    std::ostringstream out;
    (..., (out << std::forward<Args>(msgParts)));
    throw std::runtime_error(out.str());
  }

  /***********
   * Binding *
   ***********/

  void insertBinding(
      const char* typeId,
      std::any&& obj,
      BindingType bindingType,
      const std::experimental::source_location& loc) {
    const auto& [iter, inserted] = bindings.emplace(
        typeId, Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() });
    if (!inserted) {
      throwError(
          "Binding for type ",
          typeId,
          " already exists. Originally bound at ",
          iter->second.filename,
          " line ",
          iter->second.line,
          '.');
    }
  }

  /*************
   * Injection *
   *************/

  template <typename R, typename... Args>
  struct CtorInvoker;

  // These implementations require injector::inject, so they are defined in
  // src/injector/injector.hpp
  template <typename R, typename... Args>
  struct CtorInvoker<R(Args...)> {
    template <typename T>
    static T invoke();
    template <typename T>
    requires Unique<T> static T invokeImpl();
    template <typename T>
    requires Shared<T> static T invokeImpl();
    template <typename T>
    requires NonPtr<T> static T invokeImpl();
  };

  template <typename T, typename R = type_extractor_t<T>>
  requires HasInjectCtor<R>&& std::is_final_v<R> T injectByConstructorImpl() {
    return CtorInvoker<typename R::InjectCtor>::template invoke<T>();
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
      e.addClass(getId<T>());
      throw e;
    }
  }

  template <typename To>
  To extractNonPtr(Binding& binding) {
    return std::any_cast<std::function<To(void)>>(binding.obj)();
  }

  template <typename To>
  std::unique_ptr<To> extractUnique(Binding& binding) {
    return std::any_cast<std::function<std::unique_ptr<To>(void)>>(binding.obj)();
  }

  template <typename To>
  std::shared_ptr<To> extractShared(Binding& binding) {
    return std::any_cast<std::function<std::shared_ptr<To>(void)>>(binding.obj)();
  }

  template <typename Ptr>
  requires Unique<Ptr> Ptr injectImpl() {
    using T = unique_t<Ptr>;

    auto iter = bindings.find(getId<T>());
    if (iter == bindings.end()) {
      return injectByConstructor<T, Ptr>();
    }

    Binding& binding = iter->second;
    switch (binding.type) {
      case BindingType::NON_PTR:
        return std::make_unique<T>(extractNonPtr<T>(binding));
      case BindingType::UNIQUE:
        return extractUnique<T>(binding);
      case BindingType::SHARED:
        return std::make_unique<T>(*extractShared<T>(binding));
      case BindingType::IMPL:;
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename Ptr>
  requires Shared<Ptr> Ptr injectImpl() {
    using T = shared_t<Ptr>;

    auto iter = bindings.find(getId<T>());
    if (iter == bindings.end()) {
      return injectByConstructor<T, Ptr>();
    }

    Binding& binding = iter->second;
    switch (binding.type) {
      case BindingType::NON_PTR:
        return std::make_shared<T>(extractNonPtr<T>(binding));
      case BindingType::UNIQUE:
        // Implicit unique->shared ptr okay
        return extractUnique<T>(binding);
      case BindingType::SHARED:
        return extractShared<T>(binding);
      case BindingType::IMPL:;
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename T>
  requires NonPtr<T> T injectImpl() {
    using decT = std::decay_t<T>;
    auto iter = bindings.find(getId<T>());
    if (iter == bindings.end()) {
      return injectByConstructor<T, T>();
    }

    Binding& binding = iter->second;
    switch (binding.type) {
      case BindingType::NON_PTR:
        return extractNonPtr<decT>(binding);
      case BindingType::UNIQUE:
        return *extractUnique<decT>(binding);
      case BindingType::SHARED:
        return *extractShared<decT>(binding);
      case BindingType::IMPL:;
    }

    throw std::runtime_error(CTRL_PATH);
  }

  template <typename R, typename... Args>
  template <typename T>
  T CtorInvoker<R(Args...)>::invoke() {
    return invokeImpl<T>();
  }

  template <typename R, typename... Args>
  template <typename T>
  requires Unique<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return std::make_unique<R>(injectImpl<Args>()...);
  }

  template <typename R, typename... Args>
  template <typename T>
  requires Shared<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return std::make_shared<R>(injectImpl<Args>()...);
  }

  template <typename R, typename... Args>
  template <typename T>
  requires NonPtr<T> T CtorInvoker<R(Args...)>::invokeImpl() {
    return R(injectImpl<Args>()...);
  }

}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
