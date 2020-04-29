#pragma once

#include <random>
#include <tuple>
#include <type_traits>

#include "ecole/abstract.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/utility/type_traits.hpp"

namespace ecole {
namespace environment {

/**
 * Abstract class for environment Dynamics.
 *
 * A subclass defines the dynamics of the environment, that is the initial probability
 * distribution and state transition.
 * In other words, it defines an environment wihtout observations or rewards.
 * This class is used by @ref EnvironmentComposer to create the final environment with
 * state functions.
 *
 * @tparam Action The type of action recived by the environment.
 * @tparam ActionSet The type used to indicate what actions are accepted on the next
 *         transtion.
 * @tparam State The state defines the state of the environment and is shared with
 *         state functions.
 *         It contains at least the SCIP @ref scip::Model
 */
template <typename Action, typename ActionSet, typename State> class EnvironmentDynamics {
public:
	virtual ~EnvironmentDynamics() = default;

	/**
	 * Reset the State to a new initial state.
	 *
	 * This method called by the environment on @ref Environment::reset.
	 */
	virtual std::tuple<bool, ActionSet> reset_dynamics(State& init_state) = 0;

	/**
	 * Transition the State.
	 *
	 * This method called by the environment on @ref Environment::step.
	 */
	virtual std::tuple<bool, ActionSet>
	step_dynamics(State& state, Action const& action) = 0;
};

template <
	typename Dynamics,
	typename ObservationFunction,
	typename RewardFunction,
	typename TerminationFunction>
class EnvironmentComposer :
	public Environment<
		typename Dynamics::Action,
		typename Dynamics::ActionSet,
		typename ObservationFunction::Observation>,
	private Dynamics {
public:
	using Observation = typename ObservationFunction::Observation;
	using Action = typename Dynamics::Action;
	using State = typename Dynamics::State;
	using ActionSet = typename Dynamics::ActionSet;

	/**
	 * User facing constructor for the Environment.
	 */
	EnvironmentComposer(
		ObservationFunction obs_func,
		RewardFunction reward_func,
		TerminationFunction term_func) :
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
			std::tie(done, action_set) = reset_dynamics(state());
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
			std::tie(done, action_set) = step_dynamics(state(), action);
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
	using Dynamics::reset_dynamics;
	using Dynamics::step_dynamics;

	/**
	 * Getter methods to access attributes regardless of whether they are in a container.
	 */
	auto& state() { return m_state; }
	auto& obs_func() { return m_obs_func; }
	auto& reward_func() { return m_reward_func; }
	auto& term_func() { return m_term_func; }

private:
	State m_state;
	ObservationFunction m_obs_func;
	RewardFunction m_reward_func;
	TerminationFunction m_term_func;
	std::uniform_int_distribution<Seed> seed_distrib;
	bool can_transition = false;
};

}  // namespace environment
}  // namespace ecole
