#pragma once

#include <limits>
#include <type_traits>
#include <utility>

template <typename... Types>
class variant;

struct bad_variant_access : std::exception {
  const char* what() const noexcept override {
    return "bad variant access(wrong index or type)";
  }
};

inline constexpr std::size_t variant_npos = std::numeric_limits<std::size_t>::max();

template <typename T>
struct in_place_type_t {};

template <typename T>
constexpr inline in_place_type_t<T> in_place_type{};

template <std::size_t Ind>
struct in_place_index_t {};

template <std::size_t Ind>
constexpr inline in_place_index_t<Ind> in_place_index{};

template <typename T>
constexpr std::size_t get_ind_from_type() {
  return 0;
}

template <typename T, typename U, typename... Types>
constexpr std::size_t get_ind_from_type() {
  if constexpr (std::is_same_v<T, U>)
    return 0;
  else
    return 1 + get_ind_from_type<T, Types...>();
}

template <std::size_t I, typename T>
struct variant_alternative;

template <std::size_t I, typename T, typename... Types>
struct variant_alternative<I, variant<T, Types...>> {
  using type = typename variant_alternative<I - 1, variant<Types...>>::type;
};

template <typename T, typename... Types>
struct variant_alternative<0, variant<T, Types...>> {
  using type = T;
};

template <std::size_t I, typename... Types>
struct variant_alternative<I, const variant<Types...>> {
  using type = const typename variant_alternative<I, variant<Types...>>::type;
};

template <std::size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template <typename T>
struct variant_size;

template <typename... Types>
struct variant_size<variant<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <typename... Types>
struct variant_size<const variant<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <typename T>
constexpr inline std::size_t variant_size_v = variant_size<T>::value;

namespace variant_impl {
template <typename T, typename Type>
concept valid_create = requires(T && t) {
  Type{std::forward<T>(t)};
};

template <typename T, typename Type>
concept valid_func = valid_create<T, Type[]>;

template <std::size_t N, typename T, typename Type, typename... Types>
struct get_ind_to_construct : get_ind_to_construct<N + 1, T, Types...> {
  using get_ind_to_construct<N + 1, T, Types...>::get_ind;
  constexpr std::integral_constant<std::size_t, N> get_ind(Type) requires(valid_func<T, Type>);
};

template <std::size_t N, typename T, typename Type>
struct get_ind_to_construct<N, T, Type> {
  constexpr std::integral_constant<std::size_t, N> get_ind(Type) requires(valid_func<T, Type>);
};

template <typename T, typename... Types>
using ind_to_construct = decltype(get_ind_to_construct<0, T, Types...>().get_ind(std::declval<T>()));
} // namespace variant_impl
