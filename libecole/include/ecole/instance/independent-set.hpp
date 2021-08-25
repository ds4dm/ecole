#pragma once

#include <cstddef>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT IndependentSetGenerator : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		enum struct GraphType { barabasi_albert, erdos_renyi };

		std::size_t n_nodes = 500;  // NOLINT(readability-magic-numbers)
		GraphType graph_type = GraphType::barabasi_albert;
		double edge_probability = 0.25;  // NOLINT(readability-magic-numbers)
		std::size_t affinity = 4;        // NOLINT(readability-magic-numbers)
	};

	ECOLE_EXPORT static scip::Model generate_instance(Parameters parameters, RandomEngine& random_engine);

	ECOLE_EXPORT IndependentSetGenerator(Parameters parameters, RandomEngine random_engine);
	ECOLE_EXPORT IndependentSetGenerator(Parameters parameters);
	ECOLE_EXPORT IndependentSetGenerator();

	ECOLE_EXPORT scip::Model next() override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT bool done() const override { return false; }

	[[nodiscard]] ECOLE_EXPORT Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
};

}  // namespace ecole::instance
