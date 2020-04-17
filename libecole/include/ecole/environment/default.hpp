#pragma once

#include <random>
#include <type_traits>

#include <nonstd/optional.hpp>

#include "ecole/abstract.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/utility/type_traits.hpp"

namespace ecole {
namespace environment {

template <
	typename Dynamics,
	typename ObservationFunction,
	typename RewardFunction,
	typename TerminationFunction,
	typename Action = typename Dynamics::Action,
	typename Observation = typename ObservationFunction::Observation,
	typename State = typename Dynamics::State>
class DefaultEnvironment : public Environment<Action, Observation>, Dynamics {
public:
	/**
	 * User facing constructor for the Environment.
	 */
	DefaultEnvironment(
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
	std::tuple<nonstd::optional<Observation>, bool> reset(scip::Model&& model) override {
		can_transition = true;
		try {
			// Create clean new state
			state() = State{std::move(model)};

			// Bring state to initial state and reset state functions
			auto done = this->reset_state(state());
			obs_func().reset(state());
			term_func().reset(state());
			reward_func().reset(state());

			done = done || term_func().is_done(state());
			can_transition = !done;
			if (done)
				return {nonstd::nullopt, done};
			else
				return {obs_func().get(state()), done};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<nonstd::optional<Observation>, bool>
	reset(std::string const& filename) override {
		return reset(scip::Model::from_file(filename));
	}

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<nonstd::optional<Observation>, bool>
	reset(scip::Model const& model) override {
		return reset(scip::Model{model});
	}

	/**
	 * @copydoc ecole::environment::Environment::step
	 */
	std::tuple<nonstd::optional<Observation>, Reward, bool, Info>
	step(Action const& action) override {
		if (!can_transition) throw Exception("Environment need to be reset.");
		try {
			auto done = this->step_state(state(), action);
			done = done || term_func().is_done(state());
			can_transition = !done;
			auto const reward = reward_func().get(state(), done);

			if (done)
				return {{}, reward, done, Info{}};
			else
				return {obs_func().get(state()), reward, done, Info{}};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

protected:
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
