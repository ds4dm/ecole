#pragma once

#include <map>
#include <random>
#include <tuple>
#include <type_traits>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/exception.hpp"
#include "ecole/random.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/traits.hpp"

namespace ecole::environment {

using Seed = typename RandomEngine::result_type;
using Reward = reward::Reward;
using Info = int;  // FIXME dummy type while the information is not implemented

/**
 * Environment class orchestrating environment dynamics and state functions.
 *
 * Environments are the main abstraction exposed by Ecole.
 * They characterise the Markov Decision Process task to solve.
 * The interface to environments is meant to be close to that of
 * [OpenAi Gym](https://gym.openai.com/), with some differences nontheless due to the
 * requirements of Ecole.
 *
 * @tparam Dynamics The  ecole::environment::EnvironmentDynamics driving the initial state and transition of the
 *         environment
 * @tparam ObservationFunction The  ecole::observation::ObservationFunction to extract an observation out of the
 *         current state.
 * @tparam RewardFunction The  ecole::reward::RewardFunction to extract the reward of the last transition.
 */
template <typename Dynamics, typename ObservationFunction, typename RewardFunction> class Environment {
public:
	using Observation = trait::observation_of_t<ObservationFunction>;
	using Action = trait::action_of_t<Dynamics>;
	using ActionSet = trait::action_set_of_t<Dynamics>;

	/**
	 * Default construct everything and seed environment with random value.
	 */
	Environment() : random_engine(spawn_random_engine()) {}

	/**
	 * Fully customize environment and seed environment with random value.
	 */
	template <typename... Args>
	Environment(
		ObservationFunction obs_func,
		RewardFunction reward_func,
		std::map<std::string, scip::Param> scip_params,
		Args&&... args) :
		m_dynamics(std::forward<Args>(args)...),
		m_obs_func(std::move(obs_func)),
		m_reward_func(std::move(reward_func)),
		m_scip_params(std::move(scip_params)),
		random_engine(spawn_random_engine()) {}

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
	void seed(Seed new_seed) { random_engine.seed(new_seed); }

	/**
	 * Reset the environment to the initial state on the given problem instance.
	 *
	 * Takes as input a filename or loaded model.
	 *
	 * @param new_model Passed to the EnvironmentDynamics to start a new trajectory.
	 * @param args Passed to the EnvironmentDynamics.
	 * @return An observation of the new state, or nothing on terminal states.
	 * @return An subset of actions accepted on the next transition (call to  step).
	 * @return A boolean flag indicating whether the state is terminal.
	 * @post Unless the (initial) state is also terminal, transitioning (using step) is
	 *       possible.
	 */
	template <typename... Args>
	auto reset(scip::Model&& new_model, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool> {
		can_transition = true;
		try {
			// Create clean new Model
			model() = std::move(new_model);
			model().set_params(scip_params());
			dynamics().set_dynamics_random_state(model(), random_engine);

			// Bring model to initial state and reset state functions
			auto const [done, action_set] = dynamics().reset_dynamics(model(), std::forward<Args>(args)...);
			obs_func().reset(model());
			reward_func().reset(model());

			can_transition = !done;
			auto const reward_offset = reward_func().obtain_reward(model(), done);
			return {obs_func().obtain_observation(model()), std::move(action_set), reward_offset, done};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	template <typename... Args>
	auto reset(scip::Model const& model, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool> {
		return reset(model.copy_orig(), std::forward<Args>(args)...);
	}

	template <typename... Args>
	auto reset(std::string const& filename, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool> {
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
	auto step(Action const& action, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool, Info> {
		if (!can_transition) {
			throw Exception("Environment need to be reset.");
		}
		try {
			auto const [done, action_set] = dynamics().step_dynamics(model(), action, std::forward<Args>(args)...);
			can_transition = !done;
			auto const reward = reward_func().obtain_reward(model(), done);

			return {
				obs_func().obtain_observation(model()),
				std::move(action_set),
				reward,
				done,
				Info{},
			};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	auto& dynamics() { return m_dynamics; }
	auto& model() { return m_model; }
	auto& obs_func() { return m_obs_func; }
	auto& reward_func() { return m_reward_func; }
	auto& scip_params() { return m_scip_params; }

private:
	Dynamics m_dynamics;
	scip::Model m_model;
	ObservationFunction m_obs_func;
	RewardFunction m_reward_func;
	std::map<std::string, scip::Param> m_scip_params;
	RandomEngine random_engine;
	bool can_transition = false;
};

}  // namespace ecole::environment
