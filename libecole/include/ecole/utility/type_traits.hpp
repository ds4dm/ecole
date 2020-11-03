#pragma once

#include <type_traits>

namespace ecole::utility {

/**
 * Dispatch between the type and a lvalue reference to a constant.
 *
 * Used for template fuctions where the input argument could be better taken by value (e.g. an int
 * or other arithmetic type), or const reference (e.g. a std::string).
 */
template <typename T> using value_or_const_ref_t = std::conditional_t<std::is_trivially_copyable_v<T>, T, T const&>;

/**
 * A trait to detect if a type has a dereference operator, that is behaves like a pointer.
 *
 * This is useful to write generic code where a function should have the same behaviour on a type or a pointer to that
 * type.
 */
template <typename, typename = std::void_t<>> struct has_dereference : std::false_type {};

template <typename T> struct has_dereference<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type {};

template <typename T> inline constexpr bool has_dereference_v = has_dereference<T>::value;

}  // namespace ecole::utility
