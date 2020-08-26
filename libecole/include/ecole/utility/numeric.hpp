#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ecole::utility {

template <typename...> using void_t = void;

template <typename From, typename To, typename = void_t<>> struct is_narrow_castable : std::false_type {};

/**
 * Do not narrow cast char to anything else.
 */
template <typename From> struct is_narrow_castable<From, char> : std::false_type {};
template <typename To> struct is_narrow_castable<char, To> : std::false_type {};
template <> struct is_narrow_castable<char, char> : std::true_type {};

template <typename From, typename To>
struct is_narrow_castable<
	To,
	From,
	std::enable_if_t<
		std::is_convertible<From, To>::value && std::is_convertible<To, From>::value && !std::is_same<From, char>::value &&
		!std::is_same<To, char>::value>> : std::true_type {};

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

}  // namespace ecole::utility
