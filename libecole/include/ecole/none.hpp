#pragma once

#include "ecole/export.hpp"

namespace ecole {

/**
 * Type of None, an empty type.
 *
 * This is used to represent a consistent non-use of a type.
 * For instance, when not needing an observation function, this is used as the type
 * of the observation.
 */
struct ECOLE_EXPORT NoneType {
	constexpr bool operator==(NoneType /*unused*/) const { return true; }
	constexpr bool operator!=(NoneType /*unused*/) const { return false; }
};

/**
 * A constant expression representing no value.
 */
constexpr inline NoneType None;

}  // namespace ecole
