#pragma once

#include <random>
#include <tuple>
#include <type_traits>

#include "ecole/abstract.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace environment {

template <
	typename Dynamics,
	typename ObservationFunction,
	typename RewardFunction,
	typename TerminationFunction>
class EnvironmentComposer :
	public Environment<
		typename Dynamics::Action,
		typename Dynamics::ActionSet,
		typename ObservationFunction::Observation> {
public:
	using Observation = typename ObservationFunction::Observation;
	using Action = typename Dynamics::Action;
	using State = typename Dynamics::State;
	using ActionSet = typename Dynamics::ActionSet;

	/**
	 * User facing constructor for the Environment.
	 */
	template <typename... Args>
	EnvironmentComposer(
		ObservationFunction obs_func,
		RewardFunction reward_func,
		TerminationFunction term_func,
		Args&&... args) :
		m_dynamics(std::forward<Args>(args)...),
		m_obs_func(std::move(obs_func)),
		m_reward_func(std::move(reward_func)),
		m_term_func(std::move(term_func)) {}

	/**
	 * @copydoc ecole::environment::Environment::seed
	 */
	void seed(Seed) override {}
	Seed seed() const noexcept override {
		assert(false);
		return 0;
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<Observation, ActionSet, bool> reset(scip::Model&& model) override {
		can_transition = true;
		try {
			// Create clean new state
			state() = State{std::move(model)};

			// Bring state to initial state and reset state functions
			bool done;
			ActionSet action_set;
			std::tie(done, action_set) = dynamics().reset_dynamics(state());
			obs_func().reset(state());
			term_func().reset(state());
			reward_func().reset(state());

			done = done || term_func().obtain_termination(state());
			can_transition = !done;
			return {obs_func().obtain_observation(state()), std::move(action_set), done};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<Observation, ActionSet, bool> reset(std::string const& filename) override {
		return reset(scip::Model::from_file(filename));
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<Observation, ActionSet, bool> reset(scip::Model const& model) override {
		return reset(scip::Model{model});
	}

	/**
	 * @copydoc ecole::environment::Environment::step
	 */
	std::tuple<Observation, ActionSet, Reward, bool, Info>
	step(Action const& action) override {
		if (!can_transition) throw Exception("Environment need to be reset.");
		try {
			bool done;
			ActionSet action_set;
			std::tie(done, action_set) = dynamics().step_dynamics(state(), action);
			done = done || term_func().obtain_termination(state());
			can_transition = !done;
			auto const reward = reward_func().obtain_reward(state(), done);

			return {
				obs_func().obtain_observation(state()),
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

protected:
	auto& dynamics() { return m_dynamics; }
	auto& state() { return m_state; }
	auto& obs_func() { return m_obs_func; }
	auto& reward_func() { return m_reward_func; }
	auto& term_func() { return m_term_func; }

private:
	Dynamics m_dynamics;
	State m_state;
	ObservationFunction m_obs_func;
	RewardFunction m_reward_func;
	TerminationFunction m_term_func;
	std::uniform_int_distribution<Seed> seed_distrib;
	bool can_transition = false;
};

}  // namespace environment
}  // namespace ecole
