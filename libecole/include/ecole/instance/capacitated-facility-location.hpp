#pragma once

#include <cstddef>
#include <utility>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT CapacitatedFacilityLocationGenerator : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		std::size_t n_customers = 100;                                   // NOLINT(readability-magic-numbers)
		std::size_t n_facilities = 100;                                  // NOLINT(readability-magic-numbers)
		bool continuous_assignment = true;                               // NOLINT(readability-magic-numbers)
		double ratio = 5.0;                                              // NOLINT(readability-magic-numbers)
		std::pair<int, int> demand_interval = {5, 35 + 1};               // NOLINT(readability-magic-numbers)
		std::pair<int, int> capacity_interval = {10, 160 + 1};           // NOLINT(readability-magic-numbers)
		std::pair<int, int> fixed_cost_cste_interval = {0, 90 + 1};      // NOLINT(readability-magic-numbers)
		std::pair<int, int> fixed_cost_scale_interval = {100, 110 + 1};  // NOLINT(readability-magic-numbers)
	};

	ECOLE_EXPORT static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	ECOLE_EXPORT CapacitatedFacilityLocationGenerator(Parameters parameters, RandomEngine random_engine);
	ECOLE_EXPORT CapacitatedFacilityLocationGenerator(Parameters parameters);
	ECOLE_EXPORT CapacitatedFacilityLocationGenerator();

	ECOLE_EXPORT scip::Model next() override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT bool done() const override { return false; }

	[[nodiscard]] ECOLE_EXPORT Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
