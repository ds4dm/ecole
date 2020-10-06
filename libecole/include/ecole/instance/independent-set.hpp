#pragma once

#include <cstdint>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class IndependentSetGenerator : public InstanceGenerator {
public:
	struct Parameters {
		enum GraphType { barabasi_albert, erdos_renyi };

		std::size_t n_nodes = 100;       // NOLINT(readability-magic-numbers)
		double edge_probability = 0.25;  // NOLINT(readability-magic-numbers)
		std::size_t affinity = 5;        // NOLINT(readability-magic-numbers)
		GraphType graph_type = barabasi_albert;
	};

	static scip::Model generate_instance(RandomEngine& random_engine, Parameters parameters);

	IndependentSetGenerator(RandomEngine random_engine, Parameters parameters);
	IndependentSetGenerator(Parameters parameters);
	IndependentSetGenerator();

	scip::Model next() override;
	void seed(Seed seed) override;

	[[nodiscard]] Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
