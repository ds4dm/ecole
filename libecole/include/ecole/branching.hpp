#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <tuple>

#include "ecole/base/environment.hpp"
#include "ecole/observation.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace branching {

namespace internal {

template <template <typename...> class Holder> class ReverseControl {
public:
	using lock_t = std::unique_lock<std::mutex>;

	ReverseControl() noexcept;
	ReverseControl(ReverseControl&&);
	ReverseControl(scip::Model&& model);
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

template <typename Action> class ActionSpace {
public:
	using action_t = Action;

	virtual ~ActionSpace() = default;
	virtual scip::VarProxy get(scip::Model& model, Action const& action) = 0;
	virtual std::unique_ptr<ActionSpace> clone() const = 0;
};

class Fractional : public ActionSpace<std::size_t> {
public:
	using action_t = ActionSpace::action_t;

	scip::VarProxy get(scip::Model& model, action_t const& action) override;
	std::unique_ptr<ActionSpace> clone() const override;
};

template <
	typename Action,
	typename Observation,
	template <typename...> class Holder = std::unique_ptr>
class Env : public base::Env<Action, Observation, Holder> {
public:
	using env_t = base::Env<Action, Observation, Holder>;
	using typename env_t::action_t;
	using typename env_t::info_t;
	using typename env_t::obs_t;
	using typename env_t::reward_t;
	using typename env_t::seed_t;

	template <typename T> using ptr = typename env_t::template ptr<T>;

	Env(
		ptr<ActionSpace<action_t>>&& action_space,
		ptr<base::ObservationSpace<obs_t>>&& obs_space,
		ptr<base::RewardSpace>&& reward_space,
		ptr<base::TerminationSpace>&& termination_space);
	Env(
		ptr<ActionSpace<action_t>> const& action_space,
		ptr<base::ObservationSpace<obs_t>> const& obs_space,
		ptr<base::RewardSpace> const& reward_space,
		ptr<base::TerminationSpace> const& termination_space);

private:
	ptr<ActionSpace<action_t>> action_space;
	ptr<base::ObservationSpace<obs_t>> obs_space;
	ptr<base::RewardSpace> reward_space;
	ptr<base::TerminationSpace> termination_space;
	internal::ReverseControl<Holder> solve_controller;

	inline scip::Model& model() noexcept;
	std::tuple<obs_t, bool> _reset(scip::Model&& model) override;
	std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) override;
};

template <typename A, typename O, template <typename...> class H>
Env<A, O, H>::Env(
	ptr<ActionSpace<action_t>>&& action_space,
	ptr<base::ObservationSpace<obs_t>>&& obs_space,
	ptr<base::RewardSpace>&& reward_space,
	ptr<base::TerminationSpace>&& termination_space) :
	action_space(std::move(action_space)),
	obs_space(std::move(obs_space)),
	reward_space(std::move(reward_space)),
	termination_space(std::move(termination_space)) {}

template <typename A, typename O, template <typename...> class H>
Env<A, O, H>::Env(
	ptr<ActionSpace<action_t>> const& action_space,
	ptr<base::ObservationSpace<obs_t>> const& obs_space,
	ptr<base::RewardSpace> const& reward_space,
	ptr<base::TerminationSpace> const& termination_space) :
	action_space(action_space),
	obs_space(obs_space),
	reward_space(reward_space),
	termination_space(termination_space) {}

template <typename A, typename O, template <typename...> class H>
scip::Model& Env<A, O, H>::model() noexcept {
	return solve_controller.model();
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::_reset(scip::Model&& new_model) -> std::tuple<obs_t, bool> {
	new_model.seed(this->seed());
	solve_controller = internal::ReverseControl<H>(std::move(new_model));
	solve_controller.wait();
	reward_space->reset(model());
	termination_space->reset(model());
	obs_space->reset(model());
	bool const done = solve_controller.is_done() || termination_space->is_done(model());
	auto&& obs = obs_space->get(model());
	return std::make_tuple(std::move(obs), done);
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::_step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	auto const var = action_space->get(model(), action);
	solve_controller.resume(var);
	solve_controller.wait();
	bool const done = solve_controller.is_done() || termination_space->is_done(model());
	auto const reward = reward_space->get(model(), done);
	auto&& obs = obs_space->get(model());
	return std::make_tuple(std::move(obs), reward, done, info_t{});
}

}  // namespace branching
}  // namespace ecole
