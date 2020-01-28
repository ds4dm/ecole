#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <tuple>

#include "ecole/environment/base.hpp"
#include "ecole/observation/base.hpp"
#include "ecole/reward/base.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/termination/base.hpp"

namespace ecole {
namespace branching {

namespace internal {

template <template <typename...> class Holder> class ReverseControl {
public:
	using lock_t = std::unique_lock<std::mutex>;

	template <typename T> using ptr = Holder<T>;

	ReverseControl() noexcept;
	ReverseControl(ReverseControl&&);
	ReverseControl(ptr<scip::Model>&& model);
	ReverseControl& operator=(ReverseControl&&);
	~ReverseControl();

	void wait();
	void resume(scip::VarProxy var);
	bool is_done() const noexcept;
	scip::Model& model() noexcept;

private:
	// Heap allocation of thread data and synchronization primitives to enable
	// easy move semantics of the controller.
	class ThreadControl;
	std::unique_ptr<ThreadControl> thread_control;
	std::unique_ptr<lock_t> lk_ptr;
};

}  // namespace internal

template <typename Action> class ActionFunction {
public:
	using action_t = Action;

	virtual ~ActionFunction() = default;
	virtual scip::VarProxy get(scip::Model& model, Action const& action) = 0;
	virtual std::unique_ptr<ActionFunction> clone() const = 0;
};

class Fractional : public ActionFunction<std::size_t> {
public:
	using action_t = ActionFunction::action_t;

	scip::VarProxy get(scip::Model& model, action_t const& action) override;
	std::unique_ptr<ActionFunction> clone() const override;
};

template <
	typename Action,
	typename Observation,
	template <typename...> class Holder = std::unique_ptr>
class Environment : public environment::Environment<Action, Observation, Holder> {
public:
	using env_t = environment::Environment<Action, Observation, Holder>;
	using typename env_t::action_t;
	using typename env_t::info_t;
	using typename env_t::obs_t;
	using typename env_t::reward_t;
	using typename env_t::seed_t;

	template <typename T> using ptr = typename env_t::template ptr<T>;

	Environment(
		ptr<ActionFunction<action_t>>&& action_func,
		ptr<observation::ObservationFunction<obs_t>>&& obs_func,
		ptr<reward::RewardFunction>&& reward_func,
		ptr<termination::TerminationFunction>&& termination_func);
	Environment(
		ptr<ActionFunction<action_t>> const& action_func,
		ptr<observation::ObservationFunction<obs_t>> const& obs_func,
		ptr<reward::RewardFunction> const& reward_func,
		ptr<termination::TerminationFunction> const& termination_func);

private:
	ptr<ActionFunction<action_t>> action_func;
	ptr<observation::ObservationFunction<obs_t>> obs_func;
	ptr<reward::RewardFunction> reward_func;
	ptr<termination::TerminationFunction> termination_func;
	internal::ReverseControl<Holder> solve_controller;

	inline scip::Model& model() noexcept;
	std::tuple<obs_t, bool> _reset(ptr<scip::Model>&& model) override;
	std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) override;
};

template <typename A, typename O, template <typename...> class H>
Environment<A, O, H>::Environment(
	ptr<ActionFunction<action_t>>&& action_func,
	ptr<observation::ObservationFunction<obs_t>>&& obs_func,
	ptr<reward::RewardFunction>&& reward_func,
	ptr<termination::TerminationFunction>&& termination_func) :
	action_func(std::move(action_func)),
	obs_func(std::move(obs_func)),
	reward_func(std::move(reward_func)),
	termination_func(std::move(termination_func)) {}

template <typename A, typename O, template <typename...> class H>
Environment<A, O, H>::Environment(
	ptr<ActionFunction<action_t>> const& action_func,
	ptr<observation::ObservationFunction<obs_t>> const& obs_func,
	ptr<reward::RewardFunction> const& reward_func,
	ptr<termination::TerminationFunction> const& termination_func) :
	action_func(action_func),
	obs_func(obs_func),
	reward_func(reward_func),
	termination_func(termination_func) {}

template <typename A, typename O, template <typename...> class H>
scip::Model& Environment<A, O, H>::model() noexcept {
	return solve_controller.model();
}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::_reset(ptr<scip::Model>&& new_model)
	-> std::tuple<obs_t, bool> {
	new_model->seed(this->seed());
	solve_controller = internal::ReverseControl<H>(std::move(new_model));
	solve_controller.wait();
	reward_func->reset(model());
	termination_func->reset(model());
	obs_func->reset(model());
	bool const done = solve_controller.is_done() || termination_func->is_done(model());
	auto&& obs = obs_func->get(model());
	return std::make_tuple(std::move(obs), done);
}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::_step(action_t action)
	-> std::tuple<obs_t, reward_t, bool, info_t> {
	auto const var = action_func->get(model(), action);
	solve_controller.resume(var);
	solve_controller.wait();
	bool const done = solve_controller.is_done() || termination_func->is_done(model());
	auto const reward = reward_func->get(model(), done);
	auto&& obs = obs_func->get(model());
	return std::make_tuple(std::move(obs), reward, done, info_t{});
}

}  // namespace branching
}  // namespace ecole
