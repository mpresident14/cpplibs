#ifndef BINDING_MAP_HPP
#define BINDING_MAP_HPP

#include "src/injector/util.hpp"

#include <any>
#include <optional>
#include <string>
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
    // TODO: Since unannotated bindings are really common, special case it with an using
    // 'std::optional<Binding> defaultBinding;' to avoid a second lookup
    std::unordered_map<std::string, Binding> annotatedBindings;
  };

  // TODO: Require is_base_of annotation ???
  struct DefaultAnnotation {};

  // TODO: Unit test for this class

  class BindingMap {
  public:
    void insertBinding(
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
            BindingsforType{ std::unordered_map<std::string, Binding>{
                { annotationId,
                  Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() } } } });
        return;
      }

      // Check for bindings with annoationId
      std::unordered_map<std::string, Binding>& annotatedBindings = bIter->second.annotatedBindings;
      const auto& [abIter, inserted] = annotatedBindings.emplace(
          typeId, Binding{ bindingType, std::move(obj), loc.file_name(), loc.line() });
      if (inserted) {
        return;
      }

      if (annotationId == getId<DefaultAnnotation>()) {
        throwError(
            "Unannotated binding for type ",
            typeId,
            " already exists. Originally bound at ",
            abIter->second.filename,
            " line ",
            abIter->second.line,
            '.');
      }

      throwError(
          "Binding for type ",
          typeId,
          " annotated with ",
          annotationId,
          " already exists. Originally bound at ",
          abIter->second.filename,
          " line ",
          abIter->second.line,
          '.');
    }

    std::optional<Binding*> find(std::string typeId, std::string annotationId) {
      auto bIter = bindings.find(typeId);
      if (bIter == bindings.end()) {
        return {};
      }

      auto annotatedBindings = bIter->second.annotatedBindings;
      auto abIter = annotatedBindings.find(annotationId);
      if (abIter == annotatedBindings.end()) {
        return {};
      }

      return { &abIter->second };
    }

    void clearBindings() { bindings.clear(); }


  private:
    std::unordered_map<std::string, BindingsforType> bindings;
  };

}  // namespace detail
}  // namespace injector

#endif  // BINDING_MAP_HPP
