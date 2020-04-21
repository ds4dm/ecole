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

/**
 * Abstract base class for a simpler way to build environments.
 *
 * Creating an environment by inheriting this class only requires implementing the
 * SCIP logic.
 * All exception handling, seeding, use of state functions, does not need to be
 * implemented.
 *
 * @tparam Action The type of the action accepted to transition from one state to another.
 * @tparam State The internal state of the environment, usually the SCIP Model.
 */
template <typename Action, typename State> class SimpleEnvironment { public: };

namespace internal {

using namespace nonstd;

/**
 * SFINAE utility to statically detect if a given type behaves like a pointer.
 *
 * Could be a raw pointer, a `std::unique_ptr`, a `std::shared_ptr` etc.
 */
template <typename, typename = void_t<>> struct is_pointer_like : std::false_type {};
template <typename T>
struct is_pointer_like<T, void_t<decltype(*std::declval<T>())>> : std::true_type {};

template <typename Base, typename Derived>
using is_base_of = std::is_base_of<remove_cvref_t<Base>, remove_cvref_t<Derived>>;

/**
 * Utility function to access an object, either directly, or via dereferencement if
 * contained in container.
 */
template <typename T, std::enable_if_t<!is_pointer_like<T>::value, int> = 0>
auto& dereference(T&& x) {
	return x;
}
template <typename T, std::enable_if_t<is_pointer_like<T>::value, int> = 0>
auto& dereference(T&& x) {
	return *x;
}

template <typename ObservationFunction>
using observation_t = decltype(
	internal::dereference(std::declval<ObservationFunction>()).get(std::declval<State>()));

}  // namespace internal

/**
 * Default implmentation for building environments.
 *
 * The class takes care of calling the different state functions, manages exceptions,
 * and states.
 *
 * By inheriting from this class, one can create environments only providing the core
 * SCIP logic.
 *
 * All template parameters can be given directly, or as pointer to their abstract base
 * classes.
 *
 * @tparam Action As specified by the derived class.
 * @tparam State As specified by the derived class.
 * @tparam ObservationFunction
 * @tparam RewardFunction
 * @tparam TerminationFunction
 * @tparam Observation As returned from @ref reset and @ref step.
 * @tparam StateHolder If the way to store a State is different from what the derived
 *         class is expecting.
 */
template <
	typename Action_,
	typename State,
	typename ObservationFunction,
	typename RewardFunction,
	typename TerminationFunction,
	typename StateHolder = State>
class DefaultEnvironment :
	public Environment<Action_, internal::observation_t<ObservationFunction>> {
public:
	using Action = Action_;
	using Observation = internal::observation_t<ObservationFunction>;

	/**
	 * User facing constructor for the Environment.
	 *
	 * @tparam ObsFunc Cannot be set. The template is used to use universal references and
	 *                 enable both rvalues and lvalues references at once.
	 * @tparam RewFunc Same as @p ObsFunc.
	 * @tparam TermFunc Same as @p ObsFunc.
	 */
	template <
		typename ObsFunc = ObservationFunction,
		typename RewFunc = RewardFunction,
		typename TermFunc = TerminationFunction>
	DefaultEnvironment(ObsFunc&& obs_func, RewFunc&& reward_func, TermFunc&& term_func) :
		m_obs_func(std::forward<ObsFunc>(obs_func)),
		m_reward_func(std::forward<RewFunc>(reward_func)),
		m_term_func(std::forward<TermFunc>(term_func)),
		random_engine(static_cast<std::mt19937::result_type>(time(NULL))),
		seed_distrib(scip::min_seed, scip::max_seed) {
			scip_seed = seed_distrib(random_engine);
        }

	/**
	 * @copydoc ecole::environment::Environment::seed
	 */
	void seed(Seed new_seed) override {
		scip_seed = new_seed;
		random_engine.seed(static_cast<std::mt19937::result_type>(new_seed));
	}
	Seed seed() const noexcept override { return scip_seed; }

	/**
	 * @copydoc ecole::environment::Environment::reset
	 */
	std::tuple<nonstd::optional<Observation>, bool> reset(scip::Model&& model) override {
		can_transition = true;
		try {
			// Create clean new state
			del_state(state());  // FIXME issue #24
			state() = State{std::move(model)};
			seed_state(state(), scip_seed);
			scip_seed = seed_distrib(random_engine);

			// Bring state to initial state and reset state functions
			auto done = reset_state(state());
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
			auto done = step_state(state(), action);
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
	 * The environment method to seed the state.
	 *
	 * This method is called on @ref reset with a new internal seed.
	 * That way, the enrionment can deterministicly generates multiple transitions,
	 * without using the same seed on every trajectory.
	 *
	 * By default, agressively set permutation of rows and columns, LP, and Scip seed.
	 *
	 * @warning The seed passed here is an internal seed different from the one given
	 *          to @ref seed.
	 *
	 * @param blank_state An initialized state, before any instance is loaded.
	 * @param new_seed the internal seed use to seed the state, such the SCIP seed.
	 *        The seed given is in a range compatible with SCIP.
	 */
	virtual void seed_state(State& blank_state, Seed new_seed) {
		blank_state.model.set_param("randomization/permuteconss", true);
		blank_state.model.set_param("randomization/permutevars", true);
		blank_state.model.set_param("randomization/permutationseed", new_seed);
		blank_state.model.set_param("randomization/randomseedshift", new_seed);
		blank_state.model.set_param("randomization/lpseed", new_seed);
	}

	/**
	 * The environment core logic for reseting an environemnt.
	 *
	 * The method recieve a clean initialized state, seeded, and with problem loaded.
	 *
	 * @param init_state The state fully initialized, but without any logic apply to it
	 * @return Whether the environment can logically go any further.
	 *         This will be combined witht the early TerminationFunction to provide the
	 *         final `done` flag for the user.
	 */
	virtual bool reset_state(State& init_state) = 0;

	/**
	 * The environment core logic for transitionning.
	 *
	 * @param state The state as it was in the last call to @ref step_state or
	 *        @ref reset_state.
	 * @param action The action passed to @ref step.
	 * @return Whether the environment can logically go any further.
	 *         This will be combined witht the early TerminationFunction to provide the
	 *         final `done` flag for the user.
	 */
	virtual bool step_state(State& state, Action const& action) = 0;

	// FIXME issue #24
	virtual void del_state(State&) {}

	/**
	 * Getter methods to access attributes regardless of whether they are in a container.
	 */
	auto& state() { return internal::dereference(m_state); }
	auto& obs_func() { return internal::dereference(m_obs_func); }
	auto& reward_func() { return internal::dereference(m_reward_func); }
	auto& term_func() { return internal::dereference(m_term_func); }

private:
	internal::remove_cvref_t<StateHolder> m_state;
	internal::remove_cvref_t<ObservationFunction> m_obs_func;
	internal::remove_cvref_t<RewardFunction> m_reward_func;
	internal::remove_cvref_t<TerminationFunction> m_term_func;
	std::mt19937 random_engine;
	Seed scip_seed;
	std::uniform_int_distribution<Seed> seed_distrib;
	bool can_transition = false;
};

}  // namespace environment
}  // namespace ecole
