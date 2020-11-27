#ifndef TYPING_HPP
#define TYPING_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

namespace injector {
namespace detail {

  template <typename Value>
  struct is_unique : std::false_type {};

  template <typename Value>
  struct is_unique<std::unique_ptr<Value>> {
    using type = std::decay_t<Value>;
    static constexpr bool value = true;
  };

  template <typename Value>
  using unique_t = typename is_unique<std::decay_t<Value>>::type;


  template <typename Value>
  struct is_shared : std::false_type {};

  template <typename Value>
  struct is_shared<std::shared_ptr<Value>> {
    using type = std::decay_t<Value>;
    static constexpr bool value = true;
  };

  template <typename Value>
  using shared_t = typename is_shared<std::decay_t<Value>>::type;


  template <typename ValueHolder>
  concept Unique = is_unique<std::decay_t<ValueHolder>>::value;

  template <typename ValueHolder>
  concept Shared = is_shared<std::decay_t<ValueHolder>>::value;

  template <typename ValueHolder>
  concept NonPtr = !(Unique<ValueHolder> || Shared<ValueHolder>);

  // These concepts allow implicit conversion from ptr<Base> to ptr<Derived>, but prevent implicit
  // conversion from std::unique_ptr to std::shared_ptr
  template <typename Value>
  using UniqueSupplier = std::function<std::unique_ptr<Value>(void)>;

  template <typename Value>
  using SharedSupplier = std::function<std::shared_ptr<Value>(void)>;

  template <typename Value>
  using NonPtrSupplier = std::function<Value(void)>;

  template <typename Value, typename Fn>
  concept IsUniqueSupplier = std::is_convertible_v<Fn, UniqueSupplier<Value>>;

  template <typename Value, typename Fn>
  concept IsSharedSupplier =
      std::is_convertible_v<Fn, SharedSupplier<Value>> && !IsUniqueSupplier<Value, Fn>;

  template <typename Value, typename Fn>
  concept IsNonPtrSupplier =
      std::is_convertible_v<
          Fn,
          NonPtrSupplier<Value>> && !(IsUniqueSupplier<Value, Fn> || IsSharedSupplier<Value, Fn>);


  // Extracts and decays the type of the value from a shared or unique pointer
  template <typename ValueHolder>
  struct value_extractor {
    using type = std::conditional_t<
        Unique<ValueHolder>,
        unique_t<ValueHolder>,
        std::conditional_t<Shared<ValueHolder>, shared_t<ValueHolder>, std::decay_t<ValueHolder>>>;
  };
  template <typename ValueHolder>
  using value_extractor_t = typename value_extractor<ValueHolder>::type;

  template <typename Key, typename Value>
  concept Bindable = std::is_convertible_v<Value, Key>;


  // Calcuates the number of arguments to a function
  template <typename R, typename... Args>
  struct num_args;

  template <typename R, typename... Args>
  struct num_args<R(Args...)> {
    static constexpr int value = sizeof...(Args);
  };

  template <typename Fn>
  constexpr int num_args_v = num_args<Fn>::value;

  template <typename C>
  concept HasInjectCtor = requires {
    typename C::InjectCtor;
  };

  template <typename C>
  concept HasInjectAnnotations = requires {
    typename C::InjectAnnotations;
  };


  // Extracts a tuple of annotations from the ANNOTATIONS macro if it exists, otherwise an empty
  // tuple
  template <typename C, bool HasAnnotations>
  struct annotation_tuple;

  template <typename C>
  struct annotation_tuple<C, true> {
    using type = typename C::InjectAnnotations;
  };

  template <typename C>
  struct annotation_tuple<C, false> {
    using type = std::tuple<>;
  };

  template <typename C>
  using annotation_tuple_t = typename annotation_tuple<C, HasInjectAnnotations<C>>::type;


  // Computes the new tuple type after appending N instances of T to the given tuple
  template <typename Tuple, typename T, size_t N>
  struct tuple_append_n;

  template <typename Tuple, typename T>
  struct tuple_append_n<Tuple, T, 0> {
    using type = Tuple;
  };

  template <typename Tuple, typename T, size_t N>
  struct tuple_append_n {
    using type = typename tuple_append_n<
        decltype(std::tuple_cat(std::declval<Tuple>(), std::declval<std::tuple<T>>())),
        T,
        N - 1>::type;
  };

  template <typename Tuple, typename T, size_t N>
  using tuple_append_n_t = typename tuple_append_n<Tuple, T, N>::type;

}  // namespace detail
}  // namespace injector

#endif  // TYPING_HPP
