#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ecole {
namespace utility {

template <typename...> using void_t = void;

template <typename To, typename From, typename = void_t<>>
struct is_static_castable : std::false_type {};

template <typename To, typename From>
struct is_static_castable<To, From, void_t<decltype(static_cast<To>(std::declval<From>()))>> :
	std::true_type {};

template <typename To, typename From>
using is_narrow_castable = std::integral_constant<
	bool,
	is_static_castable<To, From>::value && is_static_castable<From, To>::value>;

/**
 * A narrow cast raises if any numerical loss is detected.
 */
template <typename To, typename From> To narrow_cast(From val) {
	auto val_to = static_cast<To>(val);
	if (static_cast<From>(val_to) != val) {
		throw std::runtime_error("Numerical loss converting.");
	}
	return val_to;
}

}  // namespace utility
}  // namespace ecole
