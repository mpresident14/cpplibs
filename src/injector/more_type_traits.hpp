#ifndef PREZ_INJECTOR_MORE_TYPE_TRAITS_HPP
#define PREZ_INJECTOR_MORE_TYPE_TRAITS_HPP

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

namespace prez {
namespace injector {
namespace detail {

template <typename Key>
struct is_unique : std::false_type {};

template <typename Key>
struct is_unique<std::unique_ptr<Key>> {
  using type = Key;
  static constexpr bool value = true;
};

template <typename Key>
using unique_t = typename is_unique<Key>::type;

template <typename Key>
struct is_shared : std::false_type {};

template <typename Key>
struct is_shared<std::shared_ptr<Key>> {
  using type = Key;
  static constexpr bool value = true;
};

template <typename Key>
using shared_t = typename is_shared<Key>::type;

template <typename KeyHolder>
concept Unique = is_unique<KeyHolder>::value;

template <typename KeyHolder>
concept Shared = is_shared<KeyHolder>::value;

template <typename KeyHolder>
concept NonPtr = !(Unique<KeyHolder> || Shared<KeyHolder>);

// These concepts allow implicit conversion from ptr<Base> to ptr<Derived>, but
// prevent implicit conversion from std::unique_ptr to std::shared_ptr
template <typename Key>
using UniqueSupplier = std::function<std::unique_ptr<Key>(void)>;

template <typename Key>
using SharedSupplier = std::function<std::shared_ptr<Key>(void)>;

template <typename Key>
using NonPtrSupplier = std::function<Key(void)>;

template <typename Key, typename Fn>
concept IsUniqueSupplier = std::is_convertible_v<Fn, UniqueSupplier<Key>>;

template <typename Key, typename Fn>
concept IsSharedSupplier =
    std::is_convertible_v<Fn, SharedSupplier<Key>> && !IsUniqueSupplier<Key, Fn>;

template <typename Key, typename Fn>
concept IsNonPtrSupplier = std::is_convertible_v<Fn, NonPtrSupplier<Key>> &&
    !(IsUniqueSupplier<Key, Fn> || IsSharedSupplier<Key, Fn>);

// Extracts the type of the value from a shared or unique pointer
template <typename Holder>
struct type_extractor {
  // using HolderDec = holder_decay_t<Holder>;
  using type = std::conditional_t<
      Unique<Holder>,
      unique_t<Holder>,
      std::conditional_t<Shared<Holder>, shared_t<Holder>, Holder>>;
};
template <typename HolderCVRef>
using type_extractor_t = typename type_extractor<std::decay_t<HolderCVRef>>::type;

template <typename T>
concept IsDecayed = std::is_same_v<T, std::decay_t<T>>;

template <typename Key>
concept ValidKey = !(std::is_reference_v<Key> || std::is_volatile_v<Key>);

template <typename Key, typename Value>
concept Bindable =
    // is_same needed for the case where Key = Value but is not copy nor move
    // constructible
    ValidKey<Key>&& ValidKey<Value> &&
    (std::is_convertible_v<Value, Key> || std::is_same_v<Value, Key>);

template <typename Key, typename Value>
concept ImplBindable = IsDecayed<Key>&& IsDecayed<Value>&& std::is_base_of_v<Key, Value>;

// Calcuates the number of arguments to a function
template <typename R, typename... Args>
struct num_args;

template <typename R, typename... Args>
struct num_args<R(Args...)> {
  static constexpr int value = sizeof...(Args);
};

template <typename Fn>
constexpr int num_args_v = num_args<Fn>::value;

// Determines whether a class has an injection constructor and annotations
template <typename C>
concept HasInjectCtor = requires {
  typename C::InjectCtor;
};

template <typename C>
concept HasInjectAnnotations = requires {
  typename C::InjectAnnotations;
};

// Extracts a tuple of annotations from the ANNOTATIONS macro if it exists,
// otherwise an empty tuple
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

// Computes the new tuple type after appending N instances of T to the given
// tuple
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

} // namespace detail
} // namespace injector
} // namespace prez

#endif // TYPING_HPP
