#ifndef TYPING_HPP
#define TYPING_HPP

#include <functional>
#include <memory>
#include <tuple>
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
  using unique_t = typename is_unique<std::decay_t<T>>::type;


  template <typename T>
  struct is_shared : std::false_type {};

  template <typename T>
  struct is_shared<std::shared_ptr<T>> {
    using type = T;
    static constexpr bool value = true;
  };

  template <typename T>
  using shared_t = typename is_shared<std::decay_t<T>>::type;

  /************
   * Concepts *
   ************/

  template <typename Ptr>
  concept Unique = is_unique<std::decay_t<Ptr>>::value;

  template <typename Ptr>
  concept Shared = is_shared<std::decay_t<Ptr>>::value;

  template <typename Ptr>
  concept NonPtr = !(Unique<Ptr> || Shared<Ptr>);

  // These concepts allow implicit conversion from ptr<Base> to ptr<Derived>, but prevent implicit
  // conversion from std::unique_ptr to std::shared_ptr
  template <typename To>
  using UniqueSupplier = std::function<std::unique_ptr<To>(void)>;

  template <typename To>
  using SharedSupplier = std::function<std::shared_ptr<To>(void)>;

  template <typename To>
  using NonPtrSupplier = std::function<To(void)>;

  template <typename T, typename Fn>
  concept IsUniqueSupplier = std::is_convertible_v<Fn, UniqueSupplier<T>>;

  template <typename T, typename Fn>
  concept IsSharedSupplier =
      std::is_convertible_v<Fn, SharedSupplier<T>> && !IsUniqueSupplier<T, Fn>;

  template <typename T, typename Fn>
  concept IsNonPtrSupplier = std::is_convertible_v<Fn, NonPtrSupplier<T>>;
  // && !(IsUniqueSupplier<T, Fn> || IsSharedSupplier<T, Fn>);

  template <typename T>
  struct type_extractor {
    using type = std::conditional_t<
        Unique<T>,
        unique_t<T>,
        std::conditional_t<Shared<T>, shared_t<T>, std::decay_t<T>>>;
  };
  template <typename T>
  using type_extractor_t = typename type_extractor<T>::type;

  template <typename Bound, typename To>
  concept Bindable = std::same_as<Bound, To> || std::is_base_of_v<Bound, To>;

  template <typename C>
  concept HasInjectCtor = requires {
    typename C::InjectCtor;
  };

  template <typename C>
  concept HasInjectAnnotations = requires {
    typename C::InjectAnnotations;
  };

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


  // TODO: Is there a way to accomplish this without actually forcing the compiler to generate this
  // code for functions that are only used for decltype?

  // IDEA:
  // N = sizeof(Args)... - sizeof(Annotations)...
  // Append N DefaultAnnotations to std::tuple<Annotations...>

  template <size_t N, typename T, typename Tuple, std::enable_if_t<N == 0, int> = 0>
  auto appendTupleN(Tuple tup, T toAppend) {
    return tup;
  }

  template <size_t N, typename T, typename Tuple, std::enable_if_t<N != 0, int> = 0>
  auto appendTupleN(Tuple tup, T toAppend) {
    return appendTupleN<N - 1>(std::tuple_cat(tup, std::make_tuple(toAppend)), toAppend);
  }

  template <typename Tuple, typename T, size_t N>
  struct tuple_append_n {
    using type = decltype(appendTupleN<N>(std::declval<Tuple>(), std::declval<T>()));
  };

  template <typename Tuple, typename T, size_t N>
  using tuple_append_n_t = typename tuple_append_n<Tuple, T, N>::type;

}  // namespace detail
}  // namespace injector

#endif  // TYPING_HPP
