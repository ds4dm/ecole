#pragma once

#include <cstddef>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT CombinatorialAuctionGenerator : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		std::size_t n_items = 100;       // NOLINT(readability-magic-numbers)
		std::size_t n_bids = 500;        // NOLINT(readability-magic-numbers)
		unsigned int min_value = 1;      // NOLINT(readability-magic-numbers)
		unsigned int max_value = 100;    // NOLINT(readability-magic-numbers)
		double value_deviation = 0.5;    // NOLINT(readability-magic-numbers)
		double add_item_prob = 0.65;     // NOLINT(readability-magic-numbers)
		std::size_t max_n_sub_bids = 5;  // NOLINT(readability-magic-numbers)
		double additivity = 0.2;         // NOLINT(readability-magic-numbers)
		double budget_factor = 1.5;      // NOLINT(readability-magic-numbers)
		double resale_factor = 0.5;      // NOLINT(readability-magic-numbers)
		bool integers = false;
		bool warnings = false;
	};

	ECOLE_EXPORT static scip::Model generate_instance(Parameters parameters, RandomGenerator& rng);

	ECOLE_EXPORT CombinatorialAuctionGenerator(Parameters parameters, RandomGenerator rng);
	ECOLE_EXPORT CombinatorialAuctionGenerator(Parameters parameters);
	ECOLE_EXPORT CombinatorialAuctionGenerator();

	ECOLE_EXPORT scip::Model next() override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT bool done() const override { return false; }

	[[nodiscard]] ECOLE_EXPORT Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomGenerator rng;
	Parameters parameters;
};

}  // namespace ecole::instance
