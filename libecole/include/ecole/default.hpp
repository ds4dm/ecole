#pragma once

#include <variant>

#include "ecole/export.hpp"

namespace ecole {

/**
 * Type of Default, an empty type.
 *
 * This is used to tell various Ecole functions to explicitly use the default behaviour.
 * This is more explicity and less error prone than using None or optional.
 */
struct ECOLE_EXPORT DefaultType {
	constexpr bool operator==(DefaultType /*unused*/) const { return true; }
	constexpr bool operator!=(DefaultType /*unused*/) const { return false; }
};

/**
 * A constant expression representing a default behaviour.
 */
constexpr inline DefaultType Default;

/**
 * Represent a type that is either a value or a DefaultType.
 */
template <typename T> using Defaultable = std::variant<DefaultType, T>;

}  // namespace ecole
