#pragma once

#include <cstddef>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class CombinatorialAuctionGenerator : public InstanceGenerator {
public:
	struct Parameters {
		std::size_t n_items = 100;       // NOLINT(readability-magic-numbers)
		std::size_t n_bids = 500;        // NOLINT(readability-magic-numbers)
		unsigned int min_value = 1;      // NOLINT(readability-magic-numbers)
		unsigned int max_value = 100;    // NOLINT(readability-magic-numbers)
		double value_deviation = 0.5;    // NOLINT(readability-magic-numbers)
		double add_item_prob = 0.7;      // NOLINT(readability-magic-numbers)
		std::size_t max_n_sub_bids = 5;  // NOLINT(readability-magic-numbers)
		double additivity = 0.2;         // NOLINT(readability-magic-numbers)
		double budget_factor = 1.5;      // NOLINT(readability-magic-numbers)
		double resale_factor = 0.5;      // NOLINT(readability-magic-numbers)
		bool integers = false;
		bool warnings = false;
	};

	static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	CombinatorialAuctionGenerator(Parameters parameters, RandomEngine random_engine);
	CombinatorialAuctionGenerator(Parameters parameters);
	CombinatorialAuctionGenerator();

	scip::Model next() override;
	void seed(Seed seed) override;

	[[nodiscard]] Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
