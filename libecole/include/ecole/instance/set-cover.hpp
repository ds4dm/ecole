#pragma once

#include <cstddef>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT SetCoverGenerator : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		std::size_t n_rows = 500;   // NOLINT(readability-magic-numbers)
		std::size_t n_cols = 1000;  // NOLINT(readability-magic-numbers)
		double density = 0.05;      // NOLINT(readability-magic-numbers)
		int max_coef = 100;         // NOLINT(readability-magic-numbers)
	};

	ECOLE_EXPORT static scip::Model generate_instance(Parameters parameters, RandomGenerator& rng);

	ECOLE_EXPORT SetCoverGenerator(Parameters parameters, RandomGenerator rng);
	ECOLE_EXPORT SetCoverGenerator(Parameters parameters);
	ECOLE_EXPORT SetCoverGenerator();

	ECOLE_EXPORT scip::Model next() override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT bool done() const override { return false; }

	[[nodiscard]] ECOLE_EXPORT Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomGenerator rng;
	Parameters parameters;
};

}  // namespace ecole::instance
