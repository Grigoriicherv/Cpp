#pragma once

#include "variant_utility.h"
#include <type_traits>

template <typename... Types>
concept CopyConstructible = (std::is_copy_constructible_v<Types> && ...);
template <typename... Types>
concept TriviallyCopyConstructible =
    (std::is_trivially_copy_constructible_v<Types> && ...) && CopyConstructible<Types...>;
template <typename... Types>
concept NoThrowCopyConstructible = (std::is_nothrow_copy_constructible_v<Types> && ...) && CopyConstructible<Types...>;

template <typename... Types>
concept CopyAssignable = (std::is_copy_assignable_v<Types> && ...) && CopyConstructible<Types...>;
template <typename... Types>
concept TriviallyCopyAssignable = (std::is_trivially_copy_assignable_v<Types> && ...) &&
                                  TriviallyCopyConstructible<Types...> && CopyAssignable<Types...>;
template <typename... Types>
concept NoThrowCopyAssignable =
    (std::is_nothrow_copy_assignable_v<Types> && ...) && CopyAssignable<Types...> && NoThrowCopyConstructible<Types...>;

template <typename... Types>
concept MoveConstructible = (std::is_move_constructible_v<Types> && ...);
template <typename... Types>
concept TriviallyMoveConstructible =
    (std::is_trivially_move_constructible_v<Types> && ...) && MoveConstructible<Types...>;
template <typename... Types>
concept NoThrowMoveConstructible = (std::is_nothrow_move_constructible_v<Types> && ...) && MoveConstructible<Types...>;

template <typename... Types>
concept MoveAssignable = (std::is_move_assignable_v<Types> && ...) && MoveConstructible<Types...>;
template <typename... Types>
concept TriviallyMoveAssignable = (std::is_trivially_move_assignable_v<Types> && ...) &&
                                  TriviallyMoveConstructible<Types...> && MoveAssignable<Types...>;
template <typename... Types>
concept NoThrowMoveAssignable =
    (std::is_nothrow_move_assignable_v<Types> && ...) && MoveAssignable<Types...> && NoThrowMoveConstructible<Types...>;

template <typename... Types>
concept TrivDestr = (std::is_trivially_destructible_v<Types> && ...);

template <std::size_t I, typename Variant, typename... Args>
concept CreatebleVariantInd =
    (I < variant_size_v<Variant>)&&(std::is_constructible_v<variant_alternative_t<I, Variant>, Args...>);
template <std::size_t I, typename T, typename Variant, typename... Args>
concept CreatebleVariantType = (I < variant_size_v<Variant>)&&(std::is_constructible_v<T, Args...>);
