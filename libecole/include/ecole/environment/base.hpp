#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "ecole/exception.hpp"
#include "ecole/reward/base.hpp"
#include "ecole/scip/model.hpp"

using ecole::reward::Reward;

namespace ecole {
namespace environment {

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
class Environment {
public:
	using seed_t = int;
	using info_t = int;

	// Template a holder type. Enable using std::shared_ptr for Python bindings
	template <typename T> using ptr = Holder<T>;

	virtual ~Environment() = default;

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
	std::tuple<Observation, bool> reset(ptr<scip::Model>&& model);
	std::tuple<Observation, bool> reset(scip::Model&& model);
	std::tuple<Observation, bool> reset(std::string const& filename);

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
	std::tuple<Observation, Reward, bool, info_t> step(Action action);

private:
	bool can_transition = false;
	seed_t seed_v = 0;

	void mutate_seed() noexcept;
	virtual std::tuple<Observation, bool> _reset(ptr<scip::Model>&& model) = 0;
	virtual std::tuple<Observation, Reward, bool, info_t> _step(Action action) = 0;
};

/***********************************
 *  Implementation of Environment  *
 ***********************************/

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::seed(seed_t seed) noexcept -> seed_t {
	return seed_v = seed;
}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::seed() const noexcept -> seed_t {
	return seed_v;
}

template <typename A, typename O, template <typename...> class H>
std::tuple<O, bool> Environment<A, O, H>::reset(ptr<scip::Model>&& model) {
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
std::tuple<O, bool> Environment<A, O, H>::reset(scip::Model&& model) {
	return reset(std::make_unique<scip::Model>(std::move(model)));
}

template <typename A, typename O, template <typename...> class H>
std::tuple<O, bool> Environment<A, O, H>::reset(std::string const& filename) {
	return reset(scip::Model::from_file(filename));
}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::step(A action)
	-> std::tuple<O, Reward, bool, info_t> {
	if (!can_transition) throw environment::Exception("Environment need to be reset.");
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
void Environment<A, O, H>::mutate_seed() noexcept {
	++seed_v;
}

}  // namespace environment
}  // namespace ecole
