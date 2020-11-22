#ifndef INJECTOR_DETAILS_HPP
#define INJECTOR_DETAILS_HPP

#include "src/injector/typing.hpp"

#include <any>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace injector {

namespace detail {

  const char CTRL_PATH[] = "Getting rid of clang control path warning.";


  /**************
   * Containers *
   **************/

  /* Info to be stored in bindings map for some class. */
  // struct CtorInfo {
  //   std::vector<std::std::string> argTypes;
  //   size_t line;
  //   const char* filename;
  // };
  enum class BindingType { INSTANCE, UNIQUE_PROVIDER, SHARED_PROVIDER, NON_PTR_PROVIDER };
  struct Binding {
    BindingType type;
    std::any obj;
  };

  /*************
   * Variables *
   *************/

  std::unordered_map<std::string, Binding> bindings;


  /********************
   * Helper std::functions *
   ********************/

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

  // TODO: Fix this message
  template <typename T>
  void wrongBindingError(const char* requestType, const char* boundType) {
    throwError(
        "Requested injection of ",
        requestType,
        " of type ",
        getId<T>(),
        ", but found ",
        boundType,
        " binding.");
  }

  template <typename T, typename R>
  void bindToProviderImpl(std::function<R>&& provider, BindingType bindingType) {
    if (!bindings.emplace(getId<T>(), Binding{ bindingType, std::any(std::move(provider)) })
             .second) {
      // TODO: Maybe use macro so that we can output line and file of original binding
      throwError("Binding for type ", getId<T>(), " already exists.");
    }
  }


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
  requires HasInjectCtor<R>&& std::is_final_v<R> T injectByConstructor() {
    return CtorInvoker<typename R::InjectCtor>::template invoke<T>();
  }

  template <typename T, typename R = type_extractor_t<T>>
  requires(!HasInjectCtor<R>) T injectByConstructor() {
    throwError("Class ", getId<R>(), " is not bound and has no constructors for injection.");
    throw std::runtime_error(CTRL_PATH);
  }

  template <typename T>
  T extractNonPtr(Binding& binding) {
    return std::any_cast<std::function<T(void)>>(binding.obj)();
  }

  template <typename T>
  std::unique_ptr<T> extractUnique(Binding& binding) {
    return std::any_cast<std::function<std::unique_ptr<T>(void)>>(binding.obj)();
  }

  template <typename T>
  std::shared_ptr<T> extractShared(Binding& binding) {
    return std::any_cast<std::function<std::shared_ptr<T>(void)>>(binding.obj)();
  }

  template <typename T>
  std::shared_ptr<T> extractInstance(Binding& binding) {
    return std::any_cast<std::shared_ptr<T>>(binding.obj);
  }


}  // namespace detail
}  // namespace injector

#endif  // INJECTOR_DETAILS_HPP
