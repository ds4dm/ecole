#pragma once

#include <map>
#include <random>
#include <tuple>
#include <type_traits>

#include "ecole/data/parser.hpp"
#include "ecole/dynamics/dynamics.hpp"
#include "ecole/exception.hpp"
#include "ecole/information/abstract.hpp"
#include "ecole/random.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/traits.hpp"

namespace ecole::environment {

/**
 * Environment class orchestrating environment dynamics and state functions.
 *
 * Environments are the main abstraction exposed by Ecole.
 * They characterise the Markov Decision Process task to solve.
 * The interface to environments is meant to be close to that of
 * [OpenAi Gym](https://gym.openai.com/), with some differences nontheless due to the
 * requirements of Ecole.
 *
 * @tparam Dynamics The ecole::environment::EnvironmentDynamics driving the initial state and transition of the
 *         environment
 * @tparam ObservationFunction The ecole::observation::ObservationFunction to extract an observation out of the
 *         current state.
 * @tparam RewardFunction The ecole::reward::RewardFunction to extract the reward of the last transition.
 * @tparam InformationFunction The ecole::information::InformationFunction to extract additional informations.
 */
template <typename Dynamics, typename ObservationFunction, typename RewardFunction, typename InformationFunction>
class Environment {
public:
	using Seed = ecole::Seed;
	using Observation = trait::observation_of_t<ObservationFunction>;
	using Action = trait::action_of_t<Dynamics>;
	using ActionSet = trait::action_set_of_t<Dynamics>;
	using Reward = reward::Reward;
	using Information = trait::information_of_t<InformationFunction>;
	using InformationMap = information::InformationMap<Information>;

	/**
	 * Default construct everything and seed environment with random value.
	 */
	Environment() : the_random_engine(spawn_random_engine()) {}

	/**
	 * Fully customize environment and seed environment with random value.
	 */
	template <typename... Args>
	Environment(
		ObservationFunction observation_function = {},
		RewardFunction reward_function = {},
		InformationFunction information_function = {},
		std::map<std::string, scip::Param> scip_params = {},
		Args&&... args) :
		the_dynamics(std::forward<Args>(args)...),
		the_observation_function(data::parse(std::move(observation_function))),
		the_reward_function(data::parse(std::move(reward_function))),
		the_information_function(data::parse(std::move(information_function))),
		the_scip_params(std::move(scip_params)),
		the_random_engine(spawn_random_engine()) {}

	/**
	 * Set the random seed for the environment, hence making its internals deterministic.
	 *
	 * Internally, the behavior of the environment uses a random number generator to
	 * change its behavior on every trajectroy (every call to  reset.
	 * Hence it is only required to seed the environment once.
	 *
	 * To get the same trajectory at every episode (provided the problem instance and
	 * sequence of action taken are also unchanged), one has to seed the environment before
	 * every call to reset.
	 */
	void seed(Seed new_seed) { random_engine().seed(new_seed); }

	/**
	 * Reset the environment to the initial state on the given problem instance.
	 *
	 * Takes as input a filename or loaded model.
	 *
	 * @param new_model Passed to the EnvironmentDynamics to start a new trajectory.
	 * @param args Passed to the EnvironmentDynamics.
	 * @return An observation of the new state, or nothing on terminal states.
	 * @return An subset of actions accepted on the next transition (call to  step).
	 * @return A scalar reward from the signal to maximize.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @return Any additional information about the transition.
	 * @post Unless the (initial) state is also terminal, transitioning (using step) is
	 *       possible.
	 */
	template <typename... Args>
	auto reset(scip::Model&& new_model, Args&&... args)
		-> std::tuple<Observation, ActionSet, Reward, bool, InformationMap> {
		can_transition = true;
		try {
			// Create clean new Model
			model() = std::move(new_model);
			model().set_params(scip_params());
			dynamics().set_dynamics_random_state(model(), random_engine());

			// Reset data extraction function and bring model to initial state.
			observation_function().before_reset(model());
			reward_function().before_reset(model());
			information_function().before_reset(model());
			auto const [done, action_set] = dynamics().reset_dynamics(model(), std::forward<Args>(args)...);

			can_transition = !done;
			return {
				observation_function().extract(model(), done),
				std::move(action_set),
				reward_function().extract(model(), done),
				done,
				information_function().extract(model(), done),
			};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	template <typename... Args>
	auto reset(scip::Model const& model, Args&&... args)
		-> std::tuple<Observation, ActionSet, Reward, bool, InformationMap> {
		return reset(model.copy_orig(), std::forward<Args>(args)...);
	}

	template <typename... Args>
	auto reset(std::string const& filename, Args&&... args)
		-> std::tuple<Observation, ActionSet, Reward, bool, InformationMap> {
		return reset(scip::Model::from_file(filename), std::forward<Args>(args)...);
	}

	/**
	 * Transition from one state to another.
	 *
	 * Take an action on the previously observed state and transition to a new state.
	 *
	 * @param action Passed to the EnvironmentDynamics.
	 * @param args Passed to the EnvironmentDynamics.
	 * @return An observation of the new state, or nothing on terminal states.
	 * @return An subset of actions accepted on the next transition (call to  step).
	 * @return A scalar reward from the signal to maximize.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @return Any additional information about the transition.
	 * @pre A call to reset must have been done prior to transitioning.
	 * @pre The envrionment must not be on a terminal state, or have thrown an exception.
	 *      In such cases, a call to reset must be perform before continuing.
	 */
	template <typename... Args>
	auto step(Action const& action, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool, InformationMap> {
		if (!can_transition) {
			throw Exception("Environment need to be reset.");
		}
		try {
			auto const [done, action_set] = dynamics().step_dynamics(model(), action, std::forward<Args>(args)...);
			can_transition = !done;

			return {
				observation_function().extract(model(), done),
				std::move(action_set),
				reward_function().extract(model(), done),
				done,
				information_function().extract(model(), done),
			};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	auto& dynamics() { return the_dynamics; }
	auto& model() { return the_model; }
	auto& observation_function() { return the_observation_function; }
	auto& reward_function() { return the_reward_function; }
	auto& information_function() { return the_information_function; }
	auto& scip_params() { return the_scip_params; }
	auto& random_engine() { return the_random_engine; }

private:
	Dynamics the_dynamics;
	scip::Model the_model;
	ObservationFunction the_observation_function;
	RewardFunction the_reward_function;
	InformationFunction the_information_function;
	std::map<std::string, scip::Param> the_scip_params;
	RandomEngine the_random_engine;
	bool can_transition = false;
};

}  // namespace ecole::environment
