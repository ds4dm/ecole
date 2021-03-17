#pragma once

#include <cstddef>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class IndependentSetGenerator : public InstanceGenerator {
public:
	struct Parameters {
		enum struct GraphType { barabasi_albert, erdos_renyi };

		std::size_t n_nodes = 500;  // NOLINT(readability-magic-numbers)
		GraphType graph_type = GraphType::barabasi_albert;
		double edge_probability = 0.25;  // NOLINT(readability-magic-numbers)
		std::size_t affinity = 4;        // NOLINT(readability-magic-numbers)
	};

	static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	IndependentSetGenerator(Parameters parameters, RandomEngine random_engine);
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
