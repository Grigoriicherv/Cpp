#pragma once

#include "variant_utility.h"
#include <utility>

namespace variant_impl {
template <bool triv, typename... Types>
union variant_union {
  constexpr void reset(std::size_t) {}
  constexpr variant_union() noexcept {}
  template <std::size_t Ind, typename... Args>
  constexpr variant_union(in_place_index_t<Ind>, Args&&...) {}
  template <std::size_t Ind, typename... Args>
  constexpr void emplace(in_place_index_t<Ind>, Args&&...) {}
  template <std::size_t Ind, typename T>
  constexpr void assigment(in_place_index_t<Ind>, T&&) {}
};

template <typename Head, typename... Tail>
union variant_union<false, Head, Tail...> {
  Head head;
  variant_union<false, Tail...> tail;

  constexpr variant_union() noexcept {}

  template <std::size_t Ind, typename... Args>
  constexpr variant_union(in_place_index_t<Ind>, Args&&... args)
      : tail(in_place_index<Ind - 1>, std::forward<Args>(args)...) {}

  template <typename... Args>
  constexpr variant_union(in_place_index_t<0>, Args&&... args) : head(std::forward<Args>(args)...) {}

  template <std::size_t Ind, typename... Args>
  constexpr decltype(auto) emplace(in_place_index_t<Ind>, Args&&... args) {
    return tail.emplace(in_place_index<Ind - 1>, std::forward<Args>(args)...);
  }

  template <typename... Args>
  constexpr decltype(auto) emplace(in_place_index_t<0>, Args&&... args) {
    return new (this) Head(std::forward<Args>(args)...);
  }

  template <std::size_t Ind, typename T>
  constexpr void assigment(in_place_index_t<Ind>, T&& value) {
    tail.assigment(in_place_index<Ind - 1>, std::forward<T>(value));
  }

  template <typename T>
  constexpr void assigment(in_place_index_t<0>, T&& value) {
    head = std::forward<T>(value);
  }

  constexpr void reset(std::size_t ind) {
    if (ind == 0) {
      head.~Head();
    } else {
      tail.reset(--ind);
    }
  }

  constexpr ~variant_union() noexcept {}
};

template <typename Head, typename... Tail>
union variant_union<true, Head, Tail...> {
  Head head;
  variant_union<true, Tail...> tail;

  constexpr variant_union() noexcept {}

  template <std::size_t Ind, typename... Args>
  constexpr variant_union(in_place_index_t<Ind>, Args&&... args)
      : tail(in_place_index<Ind - 1>, std::forward<Args>(args)...) {}

  template <typename... Args>
  constexpr variant_union(in_place_index_t<0>, Args&&... args) : head(std::forward<Args>(args)...) {}

  template <std::size_t Ind, typename... Args>
  constexpr decltype(auto) emplace(in_place_index_t<Ind>, Args&&... args) {
    return tail.emplace(in_place_index<Ind - 1>, std::forward<Args>(args)...);
  }

  template <typename... Args>
  constexpr decltype(auto) emplace(in_place_index_t<0>, Args&&... args) {
    return new (this) Head(std::forward<Args>(args)...);
  }

  template <std::size_t Ind, typename T>
  constexpr void assigment(in_place_index_t<Ind>, T&& value) {
    tail.assigment(in_place_index<Ind - 1>, std::forward<T>(value));
  }

  template <typename T>
  constexpr void assigment(in_place_index_t<0>, T&& value) {
    head = std::forward<T>(value);
  }

  constexpr void reset(std::size_t) {}
};

template <std::size_t N, bool triv, typename... Types>
constexpr auto const&& get_union(variant_union<triv, Types...> const&& u) {
  return std::move(get_union<N>(const_cast<variant_union<triv, Types...>&>(u)));
}

template <std::size_t N, bool triv, typename... Types>
constexpr auto const& get_union(variant_union<triv, Types...> const& u) {
  return get_union<N>(const_cast<variant_union<triv, Types...>&>(u));
}

template <std::size_t N, bool triv, typename... Types>
constexpr auto&& get_union(variant_union<triv, Types...>&& u) {
  return std::move(get_union<N>(u));
}

template <std::size_t N, bool triv, typename... Types>
constexpr auto& get_union(variant_union<triv, Types...>& u) {
  if constexpr (N == 0)
    return u.head;
  else
    return get_union<N - 1, triv>(u.tail);
}

template <std::size_t N, bool triv, typename T>
constexpr auto& get_union(variant_union<triv, T>& u) {
  if constexpr (N == 0)
    return u.head;
  else
    throw bad_variant_access();
}
} // namespace variant_impl
