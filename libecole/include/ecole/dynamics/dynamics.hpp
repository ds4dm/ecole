#pragma once

#include <random>
#include <tuple>

#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::dynamics {

/**
 * Abstract class for environment Dynamics.
 *
 * A subclass defines the dynamics of the environment, that is the initial probability
 * distribution and state transition.
 * In other words, it defines an environment wihtout observations or rewards.
 * This class is used by  Environment to create the final environment with
 * state functions.
 *
 * @tparam Action The type of action recived by the environment.
 * @tparam ActionSet The type used to indicate what actions are accepted on the next
 *         transtion.
 */
template <typename Action, typename ActionSet> class EnvironmentDynamics {
public:
	virtual ~EnvironmentDynamics() = default;

	/**
	 * Set random elements of the dynamics for the current episode.
	 */
	virtual void set_dynamics_random_state(scip::Model& model, RandomEngine& random_engine) {
		std::uniform_int_distribution<scip::Seed> seed_distrib{scip::min_seed, scip::max_seed};

		model.set_param("randomization/permuteconss", true);
		model.set_param("randomization/permutevars", true);
		model.set_param("randomization/permutationseed", seed_distrib(random_engine));
		model.set_param("randomization/randomseedshift", seed_distrib(random_engine));
		model.set_param("randomization/lpseed", seed_distrib(random_engine));
	}

	/**
	 * Reset the Model to a new initial state.
	 *
	 * This method called by the environment on  Environment::reset.
	 */
	virtual std::tuple<bool, ActionSet> reset_dynamics(scip::Model& model) = 0;

	/**
	 * Transition the Model.
	 *
	 * This method called by the environment on Environment::step.
	 */
	virtual std::tuple<bool, ActionSet> step_dynamics(scip::Model& model, Action const& action) = 0;
};

}  // namespace ecole::dynamics
