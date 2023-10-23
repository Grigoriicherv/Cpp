#pragma once

#include "variant_union.h"
#include "variant_utility.h"

namespace variant_impl {
template <bool triv_destr, typename... Types>
struct variant_storage_t {
  variant_union<false, Types...> storage;
  std::size_t index_ = 0;

  constexpr variant_storage_t() noexcept = default;
  constexpr variant_storage_t(variant_storage_t const&) = default;
  constexpr variant_storage_t(variant_storage_t&&) = default;
  constexpr variant_storage_t& operator=(variant_storage_t const&) = default;
  constexpr variant_storage_t& operator=(variant_storage_t&&) = default;

  template <std::size_t N, typename... Args>
  constexpr variant_storage_t(in_place_index_t<N>, Args&&... args)
      : storage(in_place_index<N>, std::forward<Args>(args)...), index_(N) {}

  template <std::size_t N, typename... Args>
  constexpr auto& emplace_in_storage(in_place_index_t<N>, Args&&... args) {
    index_ = variant_npos;
    auto* res = storage.emplace(in_place_index<N>, std::forward<Args>(args)...);
    index_ = N;
    return *res;
  }

  template <std::size_t N, typename T>
  constexpr void assigment_in_storage(in_place_index_t<N>, T&& value) {
    storage.assigment(in_place_index<N>, std::forward<T>(value));
    index_ = N;
  }

  constexpr void reset() {
    storage.reset(index_);
  }

  constexpr ~variant_storage_t() {
    reset();
  }
};

template <typename... Types>
struct variant_storage_t<true, Types...> {
  variant_union<true, Types...> storage;
  std::size_t index_ = 0;

  constexpr variant_storage_t() noexcept = default;
  constexpr variant_storage_t(variant_storage_t const&) = default;
  constexpr variant_storage_t(variant_storage_t&&) = default;
  constexpr variant_storage_t& operator=(variant_storage_t const&) = default;
  constexpr variant_storage_t& operator=(variant_storage_t&&) = default;

  template <std::size_t N, typename... Args>
  constexpr variant_storage_t(in_place_index_t<N>, Args&&... args)
      : storage(in_place_index<N>, std::forward<Args>(args)...), index_(N) {}

  template <std::size_t N, typename... Args>
  constexpr auto& emplace_in_storage(in_place_index_t<N>, Args&&... args) {
    index_ = variant_npos;
    auto* res = storage.emplace(in_place_index<N>, std::forward<Args>(args)...);
    index_ = N;
    return *res;
  }
  template <std::size_t N, typename T>
  constexpr void assigment_in_storage(in_place_index_t<N>, T&& value) {
    storage.assigment(in_place_index<N>, std::forward<T>(value));
    index_ = N;
  }

  constexpr void reset() {}

  ~variant_storage_t() = default;
};
} // namespace variant_impl
