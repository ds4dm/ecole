#pragma once

#include <map>
#include <random>
#include <tuple>
#include <type_traits>

#include "ecole/abstract.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/traits.hpp"

namespace ecole::environment {

template <typename Dynamics, typename ObservationFunction, typename RewardFunction>
class EnvironmentComposer :
	public Environment<
		trait::action_of_t<Dynamics>,
		trait::action_set_of_t<Dynamics>,
		trait::observation_of_t<ObservationFunction>> {
public:
	using Observation = trait::observation_of_t<ObservationFunction>;
	using Action = trait::action_of_t<Dynamics>;
	using ActionSet = trait::action_set_of_t<Dynamics>;

	/**
	 * Default construct everything and seed environment with random value.
	 */
	EnvironmentComposer() : random_engine(spawn_random_engine()) {}

	/**
	 * Fully customize environment and seed environment with random value.
	 */
	template <typename... Args>
	EnvironmentComposer(
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
	 * @copydoc ecole::environment::Environment::seed
	 */
	void seed(Seed new_seed) override { random_engine.seed(new_seed); }

	/**
	 * @copydoc ecole::environment::Environment::reset
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

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	template <typename... Args>
	auto reset(scip::Model const& model, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool> {
		return reset(model.copy_orig(), std::forward<Args>(args)...);
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	template <typename... Args>
	auto reset(std::string const& filename, Args&&... args) -> std::tuple<Observation, ActionSet, Reward, bool> {
		return reset(scip::Model::from_file(filename), std::forward<Args>(args)...);
	}

	/**
	 * @copydoc ecole::environment::Environment::step
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
