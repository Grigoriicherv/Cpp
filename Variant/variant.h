#pragma once

#include "variant_concept.h"
#include "variant_storage.h"
#include "variant_union.h"
#include "variant_utility.h"
#include "variant_visitor.h"
#include <utility>

template <typename... Types>
class variant : private variant_impl::variant_storage_t<TrivDestr<Types...>, Types...> {
  // private:
private:
  template <std::size_t I, typename... Ts>
  friend constexpr variant_alternative_t<I, variant<Ts...>>& get(variant<Ts...>& v);

public:
  // constructors
  template <typename T, std::size_t I = variant_impl::ind_to_construct<T, Types...>::value>
  constexpr variant(T&& value) noexcept(std::is_nothrow_constructible_v<variant_alternative_t<I, variant>, T>)
      : variant_impl::variant_storage_t<TrivDestr<Types...>, Types...>(in_place_index<I>, std::forward<T>(value)) {}

  constexpr variant(variant const&) requires(!CopyConstructible<Types...>) = delete;
  constexpr variant(variant const&) requires(TriviallyCopyConstructible<Types...>) = default;
  constexpr variant(variant const& other) noexcept(NoThrowCopyConstructible<Types...>)
      requires(CopyConstructible<Types...>) {
    variant_visitor::visit_ind(
        [this]<std::size_t N>(in_place_index_t<N>, variant const& other_v) {
          this->emplace_in_storage(in_place_index<N>, get_union<N>(other_v.storage));
        },
        other);
  }

  constexpr variant(variant&&) requires(!MoveConstructible<Types...>) = delete;
  constexpr variant(variant&&) requires(TriviallyMoveConstructible<Types...>) = default;
  constexpr variant(variant&& other) noexcept(NoThrowMoveConstructible<Types...>)
      requires(MoveConstructible<Types...>) {
    variant_visitor::visit_ind(
        [this]<std::size_t N>(in_place_index_t<N>, variant&& other_v) {
          this->emplace_in_storage(in_place_index<N>, std::move(get_union<N>(other_v.storage)));
        },
        std::move(other));
  }

  template <std::size_t I, typename... Args>
  constexpr variant(in_place_index_t<I>, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<variant_alternative_t<I, variant>, Args...>)
      requires(CreatebleVariantInd<I, variant, Args...>)
      : variant_impl::variant_storage_t<TrivDestr<Types...>, Types...>(in_place_index<I>, std::forward<Args>(args)...) {
  }

  template <typename T, typename... Args>
  constexpr variant(in_place_type_t<T>, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      requires(CreatebleVariantType<get_ind_from_type<T, Types...>(), T, variant, Args...>)
      : variant_impl::variant_storage_t<TrivDestr<Types...>, Types...>(in_place_index<get_ind_from_type<T, Types...>()>,
                                                                       std::forward<Args>(args)...) {}

  constexpr variant() noexcept(std::is_nothrow_default_constructible_v<variant_alternative_t<0, variant>>)
      requires(std::is_default_constructible_v<variant_alternative_t<0, variant>>)
      : variant_impl::variant_storage_t<TrivDestr<Types...>, Types...>(in_place_index<0>) {}

private:
  static constexpr void swap_move(variant& lhs, variant& rhs) noexcept(NoThrowMoveAssignable<Types...>) {
    lhs = std::move(rhs);
    rhs.reset();
    rhs.index_ = variant_npos;
  }

public:
  constexpr void swap(variant& other) noexcept(((std::is_nothrow_move_constructible_v<Types> &&
                                                 std::is_nothrow_swappable_v<Types>)&&...)) {
    if (this->valueless_by_exception() && !other.valueless_by_exception()) {
      swap_move(*this, other);
    } else if (!this->valueless_by_exception() && other.valueless_by_exception()) {
      swap_move(other, *this);
    } else if (!this->valueless_by_exception() && !other.valueless_by_exception()) {
      using std::swap;
      if (this->index() == other.index()) {
        variant_visitor::visit_ind(
            [this]<std::size_t N>(in_place_index_t<N>, variant& other_v) { swap(get<N>(*this), get<N>(other_v)); },
            other);
      } else {
        swap(*this, other);
      }
    }
  }

  // operators=
  constexpr variant& operator=(variant const&) requires(!CopyAssignable<Types...>) = delete;
  constexpr variant& operator=(variant const&) requires(TriviallyCopyAssignable<Types...>) = default;
  constexpr variant& operator=(variant const& other) noexcept(NoThrowCopyAssignable<Types...>)
      requires(CopyAssignable<Types...>) {
    variant_visitor::visit_ind(
        [this]<std::size_t N>(in_place_index_t<N>, variant const& other_v) {
          if (this->index() == other_v.index()) {
            this->assigment_in_storage(in_place_index<N>, get_union<N>(other_v.storage));
          } else {
            using rhs_type = variant_alternative_t<N, variant>;
            if constexpr (std::is_nothrow_copy_constructible_v<rhs_type> ||
                          std::is_nothrow_move_constructible_v<rhs_type>) {
              this->emplace<N>(get<N>(other_v));
            } else {
              this->operator=(variant(other_v));
            }
          }
        },
        other);
    return *this;
  }

  constexpr variant& operator=(variant&&) requires(!MoveAssignable<Types...>) = delete;
  constexpr variant& operator=(variant&&) requires(TriviallyMoveAssignable<Types...>) = default;
  constexpr variant& operator=(variant&& other) noexcept(NoThrowMoveAssignable<Types...>)
      requires(MoveAssignable<Types...>) {
    variant_visitor::visit_ind(
        [this]<std::size_t N>(in_place_index_t<N>, variant&& other_v) {
          if (this->index() == other_v.index()) {
            this->assigment_in_storage(in_place_index<N>, std::move(get_union<N>(other_v.storage)));
          } else {
            this->reset();
            this->emplace_in_storage(in_place_index<N>, std::move(get_union<N>(other_v.storage)));
          }
        },
        std::move(other));
    return *this;
  }

  template <typename T, std::size_t I = variant_impl::ind_to_construct<T, Types...>::value>
  constexpr variant&
  operator=(T&& value) noexcept(std::is_nothrow_constructible_v<variant_alternative_t<I, variant>, T>) {
    if (index() == I) {
      this->assigment_in_storage(in_place_index<I>, std::forward<T>(value));
    } else {
      if constexpr (std::is_nothrow_constructible_v<variant_alternative_t<I, variant>, T> ||
                    !std::is_nothrow_move_constructible_v<variant_alternative_t<I, variant>>) {
        emplace<I>(std::forward<T>(value));
      } else {
        emplace<I>(variant_alternative_t<I, variant>(std::forward<T>(value)));
      }
    }
    return *this;
  }

  template <typename T, typename... Args>
  constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    return emplace<get_ind_from_type<T, Types...>()>(std::forward<Args>(args)...);
  }

  template <std::size_t I, typename... Args>
  constexpr variant_alternative_t<I, variant>&
  emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<variant_alternative_t<I, variant>, Args...>) {
    this->reset();
    return this->emplace_in_storage(in_place_index<I>, std::forward<Args>(args)...);
  }

  constexpr bool valueless_by_exception() const noexcept {
    return this->index_ == variant_npos;
  }

  constexpr std::size_t index() const {
    return this->index_;
  }

  ~variant() = default;
};

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>>& get(variant<Types...>& v) {
  if (v.index() == I) {
    return get_union<I>(v.storage);
  }
  throw bad_variant_access();
}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>> const& get(variant<Types...> const& v) {
  return get<I>(const_cast<variant<Types...>&>(v));
}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>>&& get(variant<Types...>&& v) {
  return std::move(get<I>(v));
}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>> const&& get(variant<Types...> const&& v) {
  return std::move(get<I>(const_cast<variant<Types...>&>(v)));
}

template <typename T, typename... Types>
constexpr decltype(auto) get(variant<Types...>& v) {
  return get<get_ind_from_type<T, Types...>()>(v);
}

template <typename T, typename... Types>
constexpr decltype(auto) get(variant<Types...> const& v) {
  return get<get_ind_from_type<T, Types...>()>(v);
}

template <typename T, typename... Types>
constexpr decltype(auto) get(variant<Types...>&& v) {
  return get<get_ind_from_type<T, Types...>()>(std::move(v));
}

template <typename T, typename... Types>
constexpr decltype(auto) get(variant<Types...> const&& v) {
  return get<get_ind_from_type<T, Types...>()>(std::move(v));
}

template <typename T, typename... Types>
constexpr bool holds_alternative(variant<Types...> const& v) {
  return get_ind_from_type<T, Types...>() == v.index();
}

template <std::size_t I, typename... Types>
constexpr std::add_pointer_t<variant_alternative_t<I, variant<Types...>>> get_if(variant<Types...>* pv) noexcept {
  return const_cast<std::add_pointer_t<variant_alternative_t<I, variant<Types...>>>>(
      get_if<I>(const_cast<const variant<Types...>*>(pv)));
}

template <std::size_t I, typename... Types>
constexpr std::add_pointer_t<std::add_const_t<variant_alternative_t<I, variant<Types...>>>>
get_if(const variant<Types...>* pv) noexcept {
  if (pv && pv->index() == I) {
    return std::addressof(get<I>(*pv));
  } else {
    return nullptr;
  }
}

template <typename T, typename... Types>
constexpr decltype(auto) get_if(variant<Types...>* pv) noexcept {
  return get_if<get_ind_from_type<T, Types...>()>(pv);
}

template <typename T, typename... Types>
constexpr decltype(auto) get_if(const variant<Types...>* pv) noexcept {
  return get_if<get_ind_from_type<T, Types...>()>(pv);
}

template <typename... Types>
constexpr bool operator==(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (lhs.index() != rhs.index())
    return false;
  if (lhs.valueless_by_exception())
    return true;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) == get<N>(rhs); },
      rhs);
}

template <typename... Types>
constexpr bool operator!=(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (lhs.index() != rhs.index())
    return true;
  if (lhs.valueless_by_exception())
    return false;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) != get<N>(rhs); },
      rhs);
}

template <typename... Types>
constexpr bool operator<(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (rhs.valueless_by_exception())
    return false;
  if (lhs.valueless_by_exception())
    return true;
  if (lhs.index() < rhs.index())
    return true;
  if (lhs.index() > rhs.index())
    return false;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) < get<N>(rhs); },
      rhs);
}

template <typename... Types>
constexpr bool operator>(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (lhs.valueless_by_exception())
    return false;
  if (rhs.valueless_by_exception())
    return true;
  if (lhs.index() > rhs.index())
    return true;
  if (lhs.index() < rhs.index())
    return false;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) > get<N>(rhs); },
      rhs);
}

template <typename... Types>
constexpr bool operator<=(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (lhs.valueless_by_exception())
    return true;
  if (rhs.valueless_by_exception())
    return false;
  if (lhs.index() < rhs.index())
    return true;
  if (lhs.index() > rhs.index())
    return false;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) <= get<N>(rhs); },
      rhs);
}

template <typename... Types>
constexpr bool operator>=(variant<Types...> const& lhs, variant<Types...> const& rhs) {
  if (rhs.valueless_by_exception())
    return true;
  if (lhs.valueless_by_exception())
    return false;
  if (lhs.index() > rhs.index())
    return true;
  if (lhs.index() < rhs.index())
    return false;
  return variant_visitor::visit_ind(
      [&lhs]<std::size_t N>(in_place_index_t<N>, variant<Types...> const& rhs) { return get<N>(lhs) >= get<N>(rhs); },
      rhs);
}
