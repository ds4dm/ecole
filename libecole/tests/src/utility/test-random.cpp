#include <set>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/random.hpp"
#include "ecole/utility/random.hpp"

using namespace ecole;

template <typename T> auto all_different(std::vector<T> const& vec) -> bool {
	auto s = std::set<T>{vec.begin(), vec.end()};
	return s.size() == vec.size();
}

TEST_CASE("Choice return indices within items", "[utility]") {  // NOLINT
	auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
	auto weights = std::vector<double>{1., 2., 1., 3.};  // NOLINT(readability-magic-numbers)

	std::size_t const n_samples = GENERATE(0UL, 1UL, 2UL, 3UL, 4UL);
	auto indices = utility::arg_choice(n_samples, weights, random_engine);
	REQUIRE(all_different(indices));
	REQUIRE(indices.size() == n_samples);
	for (auto i : indices) {
		REQUIRE(i <= weights.size());
	}
}

TEST_CASE("Throw on invalid input", "[utility]") {
	auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
	auto weights = std::vector<double>{1., 2., 1., 3.};  // NOLINT(readability-magic-numbers)
	std::size_t const n_samples = weights.size() + 1;
	REQUIRE_THROWS_AS(utility::arg_choice(n_samples, weights, random_engine), std::invalid_argument);
}

TEST_CASE("Null weighted items are never selected", "[utility]") {  // NOLINT
	auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
	auto weights = std::vector<double>{0., 2., 1., 3.};  // NOLINT(readability-magic-numbers)

	std::size_t const n_samples = GENERATE(0UL, 1UL, 2UL, 3UL);
	std::size_t constexpr n_trials = 100;
	for (std::size_t trial = 0; trial < n_trials; ++trial) {
		auto indices = utility::arg_choice(n_samples, weights, random_engine);
		REQUIRE(std::find(indices.begin(), indices.end(), 0) == indices.end());
	}
}
