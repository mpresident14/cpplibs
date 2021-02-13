#ifndef PREZ_STREAMS_TYPING_HPP
#define PREZ_STREAMS_TYPING_HPP

#include <tuple>
#include <type_traits>
#include <utility>

// TODO: Move most of this into a library of concepts and type traits
namespace prez {
namespace streams {
  namespace detail {

    template <typename Iter>
    using iter_val_t = std::remove_cvref_t<decltype(*std::declval<Iter>())>;

    template <typename T>
    using remove_ref_wrap_t = std::remove_cvref_t<std::unwrap_reference_t<T>>;

    template <typename T>
    concept DistinctableBySort = requires(T t1, T t2) {
      { t1 < t2 }
      ->std::same_as<bool>;
      { t1 == t2 }
      ->std::same_as<bool>;
    };

    template <typename T>
    concept DistinctableByHash = requires(T t1, T t2) {
      { std::hash<T>{}(t1) }
      ->std::convertible_to<std::size_t>;
      { t1 == t2 }
      ->std::same_as<bool>;
    };

    template <typename T>
    concept Distinctable = DistinctableBySort<T> || DistinctableByHash<T>;

    // template <typename Tuple, typename IfEmpty, typename Else>
    // using cond_tuple_empty_t = std::conditional_t<std::tuple_size_v<Tuple> == 0, IfEmpty, Else>;

    // template <typename PrevTuple>
    // struct first_arg {
    //   using type = std::tuple_element_t<0, PrevTuple>;
    // };

    // template <>
    // struct first_arg<std::tuple<>> {
    //   using type = std::nullptr_t;
    // };

    // template <typename PrevTuple>
    // using first_arg_t = typename first_arg<PrevTuple>::type;

    // template <typename PrevTuple>
    // struct last_arg {
    //   using type = std::tuple_element_t<std::tuple_size_v<PrevTuple> - 1, PrevTuple>;
    // };

    // template <>
    // struct last_arg<std::tuple<>> {
    //   using type = std::nullptr_t;
    // };

    // template <typename PrevTuple>
    // using last_arg_t = typename last_arg<PrevTuple>::type;

    // template <typename Tuple, typename IndexSeq>
    // struct subtuple;

    // template <typename Tuple, size_t... Is>
    // struct subtuple<Tuple, std::index_sequence<Is...>> {
    //   using type = std::tuple<std::tuple_element_t<Is, Tuple>...>;
    // };

    // template <size_t... Is>
    // struct subtuple<std::tuple<>, std::index_sequence<Is...>> {
    //   using type = std::tuple<>;
    // };

    // template <typename Tuple, size_t Trim>
    // using subtuple_t = typename subtuple<
    //     Tuple,
    //     std::make_index_sequence<
    //         std::tuple_size_v<Tuple> == 0 ? 0 : std::tuple_size_v<Tuple> - Trim>>::type;

    // template <typename Tuple>
    // using subtuple1_t = subtuple_t<Tuple, 1>;

    // template <typename Tuple, typename T>
    // using tup_append_t =
    //     decltype(std::tuple_cat(std::declval<Tuple>(), std::declval<std::tuple<T>>()));

  }  // namespace detail
}  // namespace streams
}  // namespace prez

#endif  // TYPING_HPP
