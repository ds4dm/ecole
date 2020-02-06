#pragma once

#include <type_traits>

#include "ecole/abstract.hpp"

/**
 * Backport of simple utilities from the standard library to C++14.
 */
namespace nonstd {

template <typename T> struct remove_cvref {
	using type = std::remove_cv_t<std::remove_reference_t<T>>;
};
template <typename T> using remove_cvref_t = typename remove_cvref<T>::type;

template <typename...> using void_t = void;

}  // namespace nonstd

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
template <typename Action, typename State> class SimpleEnvironment {
public:
	virtual void reset(State& state) = 0;
	virtual void step(State& state, Action&& action) = 0;
};

namespace internal {

using namespace nonstd;

/**
 * SFINAE utility to statically detect if a given type behaves like a pointer.
 *
 * Could be a raw pointer, a @ref std::unique_ptr, a @ref std::shared_ptr etc.
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
auto& derefence(T& x) {
	return x;
}
template <typename T, std::enable_if_t<is_pointer_like<T>::value, int> = 0>
auto& derefence(T& x) {
	return *x;
}

/**
 * Environment builder from @ref SimpleEnvironemnt.
 *
 * Combines a @ref SimpleEnvironemnt with the state functions to create some default
 * behaviour around the SCIP logic.
 * This enable efficient code reuse across all @ref SimpleEnvironment.
 *
 * All template parameters can be given directly, or as pointer to their abstract base
 * classes.
 *
 * @tparam ObservationFunction
 * @tparam RewardFunction
 * @tparam TerminationFunction
 * @tparam SimpleEnvironment
 */
template <
	typename ObservationFunction,
	typename RewardFunction,
	typename TerminationFunction,
	typename SimpleEnvironment,
	typename Action = typename SimpleEnvironment::Action,
	typename Observation = typename ObservationFunction::Observation,
	typename State = typename SimpleEnvironment::State>
class EnvironmentBuilder : public Environment<Action, Observation> {
public:
	/**
	 * User facing constructor for the Environment.
	 *
	 * @tparam Args Arguments for the underlying @ref SimpleEnv.
	 * @tparam ObsFunc Cannot be set. The template is used to use universal references and
	 *                 enable both rvalues and lvalues references at once.
	 * @tparam RewFunc Same as @ref ObsFunc.
	 * @tparam TermFunc Same as @ref ObsFunc.
	 */
	template <
		typename... Args,
		typename ObsFunc = ObservationFunction,
		typename RewFunc = RewardFunction,
		typename TermFunc = TerminationFunction>
	EnvironmentBuilder(
		ObsFunc&& obs_func,
		RewFunc&& reward_func,
		TermFunc&& term_func,
		Args&&... other_args) :
		simple_env(std::forward<Args>(other_args)...),
		obs_func_m(std::forward<ObsFunc>(obs_func)),
		reward_func_m(std::forward<RewFunc>(reward_func)),
		term_func_m(std::forward<TermFunc>(term_func)) {}

	/**
	 * @copydoc Environment::seed
	 */
	Seed seed(Seed seed_v) { state().model.seed(seed_v); }
	Seed seed() const noexcept { return state().model.seed(); }

	/**
	 * @copydoc Environment::reset
	 */
	std::tuple<Observation, bool> reset() {
		mutate_seed();
		try {
			simple_env.reset();
			obs_func().reset(state());
			term_func().reset(state());
			reward_func().reset(state());
			return {obs_func().get(state()), term_func().get(state())};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	/**
	 * @copydoc Environment::step
	 */
	std::tuple<Observation, Reward, bool, Info> step(Action&& action) {
		if (!can_transition) throw Exception("Environment need to be reset.");
		try {
			simple_env.step(action);
			auto&& obs = obs_func().get(state());
			bool const done = term_func().get(state());
			can_transition = !done;
			auto const reward = reward_func().get(state(), done);
			return {obs, reward, done, Info{}};
		} catch (std::exception const&) {
			can_transition = false;
			throw;
		}
	}

	/**
	 * Getter methods to access attributes regardless of whether they are in a container.
	 */
	auto& state() const { return derefence(state_m); }
	auto& obs_func() const { return derefence(obs_func_m); }
	auto& reward_func() const { return derefence(reward_func_m); }
	auto& term_func() const { return derefence(term_func_m); }

private:
	remove_cvref_t<SimpleEnvironment> simple_env;
	remove_cvref_t<State> state_m;
	remove_cvref_t<ObservationFunction> obs_func_m;
	remove_cvref_t<RewardFunction> reward_func_m;
	remove_cvref_t<TerminationFunction> term_func_m;
	bool can_transition = false;

	void mutate_seed() { seed(seed() + 1); }
};

}  // namespace internal

}  // namespace environment
}  // namespace ecole
