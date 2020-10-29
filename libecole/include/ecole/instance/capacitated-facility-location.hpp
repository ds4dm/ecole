#pragma once

#include <cstddef>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class CapacitatedFacilityLocationGenerator : public InstanceGenerator {
public:
	struct Parameters {
		std::size_t n_customers = 100;       // NOLINT(readability-magic-numbers)
		std::size_t n_facilities = 100;      // NOLINT(readability-magic-numbers)
		bool continuous_assignment = false;  // NOLINT(readability-magic-numbers)
		double ratio = 5.0;                  // NOLINT(readability-magic-numbers)
	};

	static scip::Model generate_instance(RandomEngine& random_engine, Parameters parameters);

	CapacitatedFacilityLocationGenerator(RandomEngine random_engine, Parameters parameters);
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
