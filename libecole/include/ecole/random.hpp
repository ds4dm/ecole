#pragma once

#include <random>
#include <string>

#include "ecole/export.hpp"

namespace ecole {

using RandomEngine = std::mt19937;
using Seed = RandomEngine::result_type;

/**
 * Seed the main random generator of Ecole.
 *
 * All random generatorderive from this seeding.
 * When no seeding is performed Ecole uses true randomness.
 * Seeding does not affect random engines already created.
 */
ECOLE_EXPORT auto seed(Seed val) -> void;

/**
 * Get a new random engine that derive from Ecole's main source of randomness.
 *
 * This is the function used by all Ecole components that need a random engine.
 * While the function is thread safe, undeterministic behaviour can happen if this function is call in different threads
 * in a non deterministic order.
 */
ECOLE_EXPORT auto spawn_random_engine() -> RandomEngine;

/**
 * Convert the state of the random engine to a string.
 */
ECOLE_EXPORT auto serialize(RandomEngine const& engine) -> std::string;

/**
 * Convert a string representing the state of a random engine to a random engine.
 */
ECOLE_EXPORT auto deserialize(std::string const& data) -> RandomEngine;

}  // namespace ecole
