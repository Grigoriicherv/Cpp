#pragma once

#include "variant_utility.h"
#include <array>
#include <functional>
#include <type_traits>
#include <utility>

namespace variant_visitor {
template <typename... Functions>
constexpr auto make_array(Functions&&... funcs) {
  return std::array<std::common_type_t<Functions...>, sizeof...(Functions)>{std::forward<Functions>(funcs)...};
}

template <typename Visitor, typename... Variants, std::size_t... Inds>
constexpr auto get_matrix_impl(std::index_sequence<Inds...>) noexcept {
  return [](Visitor&& visitor, Variants&&... variants) {
    return std::forward<Visitor>(visitor)(get<Inds>(std::forward<Variants>(variants))...);
  };
}

template <typename Visitor, typename... Variants, std::size_t... First_Inds, std::size_t... Second_Inds,
          typename... Other_Seq>
constexpr auto get_matrix_impl(std::index_sequence<First_Inds...>, std::index_sequence<Second_Inds...>,
                               Other_Seq... other_seq) noexcept {
  return make_array(
      get_matrix_impl<Visitor, Variants...>(std::index_sequence<First_Inds..., Second_Inds>(), other_seq...)...);
}

template <typename Visitor, typename... Variants>
constexpr auto get_matrix() noexcept {
  return get_matrix_impl<Visitor, Variants...>(
      std::index_sequence<>(), std::make_index_sequence<variant_size_v<std::remove_cvref_t<Variants>>>()...);
}

template <typename Visitor, typename... Variants>
inline constexpr auto matrix = get_matrix<Visitor, Variants...>();

template <typename Visitor, typename Variant, std::size_t Ind>
constexpr auto get_index_arr_impl_2(std::index_sequence<Ind>) noexcept {
  return [](Visitor&& visitor, Variant&& v) {
    return std::forward<Visitor>(visitor)(in_place_index<Ind>, std::forward<Variant>(v));
  };
}

template <typename Visitor, typename Variant, std::size_t... Inds>
constexpr auto get_index_arr_impl_1(std::index_sequence<Inds...>) noexcept {
  return make_array(get_index_arr_impl_2<Visitor, Variant>(std::index_sequence<Inds>())...);
}

template <typename Visitor, typename Variant>
constexpr auto get_index_arr() noexcept {
  return get_index_arr_impl_1<Visitor, Variant>(
      std::make_index_sequence<variant_size_v<std::remove_cvref_t<Variant>>>());
}

template <typename Visitor, typename Variant>
inline constexpr auto index_arr = get_index_arr<Visitor, Variant>();

template <typename Function>
constexpr decltype(auto) get_func(Function&& func) {
  return std::forward<Function>(func);
}

template <typename Matrix, typename... Inds>
constexpr decltype(auto) get_func(Matrix&& matrix, std::size_t ind, Inds... other_inds) {
  return get_func(std::forward<Matrix>(matrix)[ind], other_inds...);
}

template <typename Visitor, typename Variant>
constexpr decltype(auto) visit_ind(Visitor&& visitor, Variant&& variant) {
  return variant_visitor::get_func(variant_visitor::index_arr<Visitor, Variant>,
                                   variant.index())(std::forward<Visitor>(visitor), std::forward<Variant>(variant));
}
}; // namespace variant_visitor

template <typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&& visitor, Variants&&... variants) {
  if ((variants.valueless_by_exception() || ...)) {
    throw bad_variant_access();
  }
  return variant_visitor::get_func(variant_visitor::matrix<Visitor, Variants...>, variants.index()...)(
      std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}
