#pragma once

#include <type_traits>

/**
 * Backport of simple utilities from the standard library to C++14.
 */
namespace nonstd {

template <typename T> struct remove_cvref {
	using type = std::remove_cv_t<std::remove_reference_t<T>>;
};
template <typename T> using remove_cvref_t = typename remove_cvref<T>::type;

template <typename...> using void_t = void;

}  // namespace nonstd
