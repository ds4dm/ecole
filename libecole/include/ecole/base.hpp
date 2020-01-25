#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace base {

/**
 * Abstract base class for all observation functions.
 *
 * Observation functions can be given to environments to parametrize what observations
 * (or partially observed states) are returned at every transition.
 * An observation function is intended to extract the observation out of the scip::Model
 * in any way desired (including caching, scaling...).
 * An observation on the contrary hand is a purely self contained data class with no
 * function.
 *
 * @tparam Observation the type of the observation extracted by this class.
 */
template <typename Observation> struct ObservationFunction {
	using obs_t = Observation;

	virtual ~ObservationFunction() = default;
	virtual std::unique_ptr<ObservationFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by environments when needing to return an observation.
	 */
	virtual obs_t get(scip::Model const& model) = 0;
};

/**
 * Abstract base class for all reward functions.
 *
 * Reward functions can be given to environments to parametrize what rewards are returned
 * at every transition.
 */
struct RewardFunction {
	using reward_t = double;

	virtual ~RewardFunction() = default;
	virtual std::unique_ptr<RewardFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual reward_t get(scip::Model const& model, bool done = false) = 0;
};

/**
 * Abstract base class for all termination functions.
 *
 * Termination functions can be given to environments to parametrize when the environment
 * terminates (that is, @ref Env::step returns `true` for the `done` flag).
 */
struct TerminationFunction {
	virtual ~TerminationFunction() = default;
	virtual std::unique_ptr<TerminationFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual bool is_done(scip::Model const& model) = 0;
};

/**
 * Abstract base class for all environments.
 *
 * Environments are the main abstraction exposed by Ecole.
 * They characterise the Markov Decision Process task to solve.
 * The interface to environments is meant to be close to that of
 * [OpenAi Gym](https://gym.openai.com/), with some differences nontheless due to the
 * requirements of Ecole.
 *
 * @tparam Action The type of the action accepted to transition from one state to another.
 * @tparam Observation The type of the observation returned on every state.
 * @tparam Holder type for functions and @ref scip::Model. The default should be used.
 */
template <
	typename Action,
	typename Observation,
	template <typename...> class Holder = std::unique_ptr>
class Env {
public:
	using action_t = Action;
	using obs_t = Observation;
	using reward_t = RewardFunction::reward_t;
	using seed_t = int;
	using info_t = int;

	// Template a hodler type. Enable using std::shared_ptr for Python bindings
	template <typename T> using ptr = Holder<T>;

	virtual ~Env() = default;

	/**
	 * Set the random seed for the environment, hence making its internals deterministic.
	 *
	 * The seed is deteministically changed at every new episode (every call to reset) to
	 * avoid overfitting on a single seed.
	 * To get the same trajectory at every episode (provided the problem instance and
	 * sequence of action taken are also unchanged), one has to seed the environment before
	 * every call to reset.
	 */
	seed_t seed(seed_t seed) noexcept;

	/**
	 * Get the current random seed.
	 */
	seed_t seed() const noexcept;

	/**
	 * Reset the environment to the initial state on the given problem instance.
	 *
	 * @return An observation of the new state.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @post Unless the (initial) state is also terminal, transitioning (using step) is
	 *       possible.
	 */
	std::tuple<obs_t, bool> reset(ptr<scip::Model>&& model);
	std::tuple<obs_t, bool> reset(scip::Model&& model);
	std::tuple<obs_t, bool> reset(std::string const& filename);

	/**
	 * Transition from one state to another.
	 *
	 * Take an action on the previously observed state and transition to a new state.
	 *
	 * @param action
	 * @return An observation of the new state.
	 * @return A scalar reward from the signal to maximize.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @return Any additional information about the transition.
	 * @pre A call to reset must have been done prior to transitioning.
	 * @pre The envrionment must not be on a terminal state, or have thrown an exception.
	 *      In such cases, a call to reset must be perform before continuing.
	 */
	std::tuple<obs_t, reward_t, bool, info_t> step(action_t action);

private:
	bool can_transition = false;
	seed_t seed_v = 0;

	void mutate_seed() noexcept;
	virtual std::tuple<obs_t, bool> _reset(ptr<scip::Model>&& model) = 0;
	virtual std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) = 0;
};

template <typename O> void ObservationFunction<O>::reset(scip::Model const& model) {
	(void)model;
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::seed(seed_t seed) noexcept -> seed_t {
	return seed_v = seed;
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::seed() const noexcept -> seed_t {
	return seed_v;
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::reset(ptr<scip::Model>&& model) -> std::tuple<obs_t, bool> {
	if (model == nullptr) throw Exception("Invalid null pointer to Model");
	mutate_seed();
	try {
		auto result = _reset(std::move(model));
		can_transition = !std::get<1>(result);
		return result;
	} catch (std::exception const&) {
		can_transition = false;
		throw;
	}
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::reset(scip::Model&& model) -> std::tuple<obs_t, bool> {
	return reset(std::make_unique<scip::Model>(std::move(model)));
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::reset(std::string const& filename) -> std::tuple<obs_t, bool> {
	return reset(scip::Model::from_file(filename));
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	if (!can_transition) throw Exception("Environment need to be reset.");
	try {
		auto result = _step(std::move(action));
		can_transition = !std::get<2>(result);
		return result;
	} catch (std::exception const&) {
		can_transition = false;
		throw;
	}
}

template <typename A, typename O, template <typename...> class H>
void Env<A, O, H>::mutate_seed() noexcept {
	++seed_v;
}

}  // namespace base
}  // namespace ecole
