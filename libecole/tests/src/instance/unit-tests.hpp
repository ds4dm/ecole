#pragma once

#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::instance {

/** Check that the problem instances permutations are the same.
 *
 * Check that all constraints and variables are the same without trying any reordering.
 */
auto same_problem_permutation(scip::Model const& model1, scip::Model const& model2) noexcept -> bool;

template <typename Generator> void unit_tests(Generator&& generator) {
	using Parameters = typename Generator::Parameters;

	SECTION("Generate instance consume random engine") {
		// NOLINTNEXTLINE(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
		auto const original_random_engine = RandomEngine{};
		auto random_engine = original_random_engine;
		auto model = Generator::generate_instance(Parameters{}, random_engine);
		STATIC_REQUIRE(std::is_same_v<decltype(model), scip::Model>);
		REQUIRE(random_engine != original_random_engine);
	}

	SECTION("Has default constructor") { Generator{}; }

	SECTION("Has parameter constructor") { Generator{Parameters{}}; }

	SECTION("Has parameter and random engine constructor") {
		// NOLINTNEXTLINE(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
		Generator{Parameters{}, RandomEngine{}};
	}

	SECTION("Generate instances in loop") {
		static auto constexpr n_instances = 2;
		for (auto i = 0; i < n_instances; ++i) {
			auto model = generator.next();
		}
	}

	SECTION("Successive instances are different") {
		auto const model1 = generator.next();
		auto const model2 = generator.next();
		REQUIRE_FALSE(same_problem_permutation(model1, model2));
	}

	SECTION("Same seed give reproducible results") {
		generator.seed(0);
		auto const model1 = generator.next();
		generator.seed(0);
		auto const model2 = generator.next();
		REQUIRE(same_problem_permutation(model1, model2));
	}

	SECTION("Generated models are valid SCIP models") {
		auto model = generator.next();
		model.solve();
		REQUIRE(model.is_solved());
	}
}

}  // namespace ecole::instance
