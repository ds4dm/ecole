#pragma once

#include <random>
#include <string>

#include "ecole/export.hpp"

namespace ecole {

using RandomGenerator = std::mt19937;
using Seed = RandomGenerator::result_type;

/**
 * Seed the main random generator of Ecole.
 *
 * All random generatorderive from this seeding.
 * When no seeding is performed Ecole uses true randomness.
 * Seeding does not affect random generators already created.
 */
ECOLE_EXPORT auto seed(Seed val) -> void;

/**
 * Get a new random generator that derive from Ecole's main source of randomness.
 *
 * This is the function used by all Ecole components that need a random generator.
 * While the function is thread safe, undeterministic behaviour can happen if this function is call in different threads
 * in a non deterministic order.
 */
ECOLE_EXPORT auto spawn_random_generator() -> RandomGenerator;

/**
 * Convert the state of the random generator to a string.
 */
ECOLE_EXPORT auto serialize(RandomGenerator const& rng) -> std::string;

/**
 * Convert a string representing the state of a random generator to a random generator.
 */
ECOLE_EXPORT auto deserialize(std::string const& data) -> RandomGenerator;

}  // namespace ecole
