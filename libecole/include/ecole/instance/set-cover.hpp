#pragma once

#include <cstddef>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class SetCoverGenerator : public InstanceGenerator {
public:
	struct Parameters {
		std::size_t n_rows = 500;   // NOLINT(readability-magic-numbers)
		std::size_t n_cols = 1000;  // NOLINT(readability-magic-numbers)
		double density = 0.05;      // NOLINT(readability-magic-numbers)
		int max_coef = 100;         // NOLINT(readability-magic-numbers)
	};

	static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	SetCoverGenerator(Parameters parameters, RandomEngine random_engine);
	SetCoverGenerator(Parameters parameters);
	SetCoverGenerator();

	scip::Model next() override;
	void seed(Seed seed) override;

	[[nodiscard]] Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
