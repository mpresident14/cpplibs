#ifndef TYPING_HPP
#define TYPING_HPP

#include <memory>
#include <type_traits>

namespace injector {

namespace detail {

  /************************
   * Type trait templates *
   ************************/

  template <typename T>
  struct is_unique : std::false_type {};

  template <typename T>
  struct is_unique<std::unique_ptr<T>> {
    using type = T;
    static constexpr bool value = true;
  };

  template <typename T>
  using unique_t = typename is_unique<T>::type;


  template <typename T>
  struct is_shared : std::false_type {};

  template <typename T>
  struct is_shared<std::shared_ptr<T>> {
    using type = T;
    static constexpr bool value = true;
  };

  template <typename T>
  using shared_t = typename is_shared<T>::type;

  /************
   * Concepts *
   ************/

  template <typename Ptr>
  concept Unique = is_unique<Ptr>::value;

  template <typename Ptr>
  concept Shared = is_shared<Ptr>::value;

  template <typename Ptr>
  concept NonPtr = !(Unique<Ptr> || Shared<Ptr>);

  // These concepts allow implicit conversion from ptr<Base> to ptr<Derived>, but prevent implicit
  // conversion from std::unique_ptr to std::shared_ptr
  template <typename T, typename Fn>
  concept UniqueProvider = std::is_convertible_v<std::invoke_result_t<Fn>, std::unique_ptr<T>>;

  template <typename T, typename Fn>
  concept SharedProvider =
      std::is_convertible_v<std::invoke_result_t<Fn>, std::shared_ptr<T>> && !UniqueProvider<T, Fn>;

  template <typename T, typename Fn>
  concept NonPtrProvider = !(UniqueProvider<T, Fn> || SharedProvider<T, Fn>);

  template <typename T>
  struct type_extractor {
    using type = std::conditional_t<
        Unique<T>,
        unique_t<T>,
        std::conditional_t<Shared<T>, shared_t<T>, std::decay_t<T>>>;
  };
  template <typename T>
  using type_extractor_t = typename type_extractor<T>::type;

  template <typename C>
  concept HasInjectCtor = requires {
    typename C::InjectCtor;
  };

}  // namespace detail
}  // namespace injector

#endif  // TYPING_HPP
