#include <random>

#include "ecole/dynamics/parts.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::dynamics {

auto DefaultSetDynamicsRandomState::set_dynamics_random_state(scip::Model& model, RandomGenerator& rng) const -> void {
	std::uniform_int_distribution<scip::Seed> seed_distrib{scip::min_seed, scip::max_seed};
	model.set_param("randomization/permuteconss", true);
	model.set_param("randomization/permutevars", true);
	model.set_param("randomization/permutationseed", seed_distrib(rng));
	model.set_param("randomization/randomseedshift", seed_distrib(rng));
	model.set_param("randomization/lpseed", seed_distrib(rng));
}

}  // namespace ecole::dynamics
