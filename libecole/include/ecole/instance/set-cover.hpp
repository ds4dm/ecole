#pragma once

#include <cstdint>
#include <random>

#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::instance {

class SetCoverGenerator {
public:
	struct Parameters {
		std::size_t n_rows = 500;   // NOLINT(readability-magic-numbers)
		std::size_t n_cols = 1000;  // NOLINT(readability-magic-numbers)
		double density = 0.05;      // NOLINT(readability-magic-numbers)
		int max_coef = 100;         // NOLINT(readability-magic-numbers)
	};

	static scip::Model generate_instance(RandomEngine& random_engine, Parameters parameters);

	SetCoverGenerator(RandomEngine random_engine, Parameters parameters);
	SetCoverGenerator(Parameters parameters);
	SetCoverGenerator();

	scip::Model next();

	[[nodiscard]] Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
