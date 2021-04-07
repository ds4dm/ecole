#pragma once

#include <cstddef>
#include <utility>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class CapacitatedFacilityLocationGenerator : public InstanceGenerator {
public:
	struct Parameters {
		std::size_t n_customers = 100;                                   // NOLINT(readability-magic-numbers)
		std::size_t n_facilities = 100;                                  // NOLINT(readability-magic-numbers)
		bool continuous_assignment = true;                               // NOLINT(readability-magic-numbers)
		double ratio = 5.0;                                              // NOLINT(readability-magic-numbers)
		std::pair<int, int> demand_interval = {5, 35 + 1};               // NOLINT(readability-magic-numbers)
		std::pair<int, int> capacity_interval = {10, 160 + 1};           // NOLINT(readability-magic-numbers)
		std::pair<int, int> fixed_cost_cste_interval = {0, 90 + 1};      // NOLINT(readability-magic-numbers)
		std::pair<int, int> fixed_cost_scale_interval = {100, 110 + 1};  // NOLINT(readability-magic-numbers)
	};

	static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	CapacitatedFacilityLocationGenerator(Parameters parameters, RandomEngine random_engine);
	CapacitatedFacilityLocationGenerator(Parameters parameters);
	CapacitatedFacilityLocationGenerator();

	scip::Model next() override;
	void seed(Seed seed) override;

	[[nodiscard]] Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
