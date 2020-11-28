#ifndef BINDING_MAP_HPP
#define BINDING_MAP_HPP

#include "src/injector/util.hpp"

#include <any>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <experimental/source_location>

namespace injector {
namespace detail {

  enum class BindingType { UNIQUE, SHARED, NON_PTR, IMPL };

  struct Binding {
    BindingType type;
    std::any obj;
    const char* filename;
    size_t line;
  };

  struct BindingsforType {
    // Since unannotated bindings are much more common, we special case it using defaultBinding to
    // avoid a second hash lookup upon retrieval
    std::optional<Binding> defaultBinding;
    std::unordered_map<std::string, Binding> annotatedBindings;
  };

  class BindingMap {
  public:
    template <typename Annotation>
    void insertBinding(
        const char* typeId,
        std::any&& obj,
        BindingType bindingType,
        const std::experimental::source_location& loc) {
      if constexpr (std::is_same_v<Annotation, Unannotated>) {
        return insertDefaultBinding(typeId, std::move(obj), bindingType, loc);
      }
      return insertAnnotatedBinding(typeId, getId<Annotation>(), std::move(obj), bindingType, loc);
    }

    template <typename Annotation>
    Binding* lookupBinding(const char* typeId) {
      if constexpr (std::is_same_v<Annotation, Unannotated>) {
        return lookupDefaultBinding(typeId);
      }
      return lookupAnnotatedBinding(typeId, getId<Annotation>());
    }


    void clearBindings() { bindings.clear(); }


  private:
    void insertDefaultBinding(
        const char* typeId,
        std::any&& obj,
        BindingType bindingType,
        const std::experimental::source_location& loc) {
      // Check for any bindings
      auto bIter = bindings.find(typeId);
      if (bIter == bindings.end()) {
        bindings.emplace(
            typeId,
            BindingsforType{
                { Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() } }, {} });
        return;
      }

      // Check for default binding
      std::optional<Binding>& defaultBinding = bIter->second.defaultBinding;
      if (defaultBinding) {
        duplicateDefaultBindingError(typeId, *defaultBinding);
      }

      defaultBinding = { Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() } };
    }


    void insertAnnotatedBinding(
        const char* typeId,
        const char* annotationId,
        std::any&& obj,
        BindingType bindingType,
        const std::experimental::source_location& loc) {
      // Check for any bindings
      auto bIter = bindings.find(typeId);
      if (bIter == bindings.end()) {
        bindings.emplace(
            typeId,
            BindingsforType{
                {},
                { { annotationId,
                    Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() } } } });
        return;
      }

      // Check for bindings with annotationId
      std::unordered_map<std::string, Binding>& annotatedBindings = bIter->second.annotatedBindings;
      const auto& [abIter, inserted] = annotatedBindings.emplace(
          annotationId, Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() });
      if (!inserted) {
        duplicateAnnotatedBindingError(typeId, annotationId, abIter->second);
      }
    }


    void duplicateDefaultBindingError(const char* typeId, Binding& existingBinding) {
      throw std::runtime_error(strCat(
          "Unannotated binding for type '",
          typeId,
          "' already exists. Originally bound at ",
          existingBinding.filename,
          " line ",
          existingBinding.line,
          '.'));
    }

    void duplicateAnnotatedBindingError(
        const char* typeId, const char* annotationId, Binding& existingBinding) {
      throw std::runtime_error(strCat(
          "Binding for type '",
          typeId,
          "' annotated with type '",
          annotationId,
          "' already exists. Originally bound at ",
          existingBinding.filename,
          " line ",
          existingBinding.line,
          '.'));
    }

    // Even though some code is duplicated within the two lookup functions below, we separate it to
    // keep the templated lookupBinding function as small as possible.
    Binding* lookupDefaultBinding(const char* typeId) {
      auto bIter = bindings.find(typeId);
      if (bIter == bindings.end()) {
        return nullptr;
      }


      std::optional<Binding>& defaultBinding = bIter->second.defaultBinding;
      return defaultBinding ? &*defaultBinding : nullptr;
    }

    Binding* lookupAnnotatedBinding(const char* typeId, const char* annotationId) {
      auto bIter = bindings.find(typeId);
      if (bIter == bindings.end()) {
        return nullptr;
      }

      auto& annotatedBindings = bIter->second.annotatedBindings;
      auto abIter = annotatedBindings.find(annotationId);
      if (abIter == annotatedBindings.end()) {
        return nullptr;
      }

      return &abIter->second;
    }


    std::unordered_map<std::string, BindingsforType> bindings;
  };

}  // namespace detail
}  // namespace injector

#endif  // BINDING_MAP_HPP
