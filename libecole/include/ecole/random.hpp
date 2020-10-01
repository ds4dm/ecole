#pragma once

#include <random>

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
auto seed(Seed val) -> void;

/**
 * Get a new random engine that derive from Ecole's main source of randomness.
 *
 * This is the function used by all Ecole components that need a random engine.
 * While the function is thread safe, undeterministic behaviour can happen if this function is call in different threads
 * in a non deterministic order.
 */
auto spawn_random_engine() -> RandomEngine;

}  // namespace ecole
