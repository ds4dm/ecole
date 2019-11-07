#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <tuple>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace branching {

namespace internal {

class ReverseControl {
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
	// Heap allocation of thread data and synchronizaation primitives to enable
	// easy move semantics of the controller.
	class ThreadControl;
	std::unique_ptr<ThreadControl> thread_control;
	std::unique_ptr<lock_t> lk_ptr;
};

} // namespace internal

template <typename Action> class ActionSpace {
public:
	using action_t = Action;

	virtual scip::VarProxy get(scip::Model& model, Action const& action) = 0;
	virtual ~ActionSpace() = default;
};

class Fractional : public ActionSpace<std::size_t> {
public:
	using action_t = ActionSpace::action_t;

	scip::VarProxy get(scip::Model& model, action_t const& action);
};

template <typename Observation, typename Action>
class Env : public base::Env<Observation, Action> {
public:
	using env_t = base::Env<Observation, Action>;
	using typename env_t::action_t;
	using typename env_t::info_t;
	using typename env_t::obs_t;
	using typename env_t::reward_t;
	using typename env_t::seed_t;

	Env(
		std::unique_ptr<base::ObservationSpace<obs_t>>&& obs_space,
		std::unique_ptr<ActionSpace<action_t>>&& action_space);

private:
	std::unique_ptr<base::ObservationSpace<obs_t>> obs_space;
	std::unique_ptr<ActionSpace<action_t>> action_space;
	internal::ReverseControl solve_controller;

	inline scip::Model& model() noexcept;
	std::tuple<obs_t, bool> _reset(scip::Model model) override;
	std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) override;
};

template <typename O, typename A>
Env<O, A>::Env(
	std::unique_ptr<base::ObservationSpace<obs_t>>&& obs_space,
	std::unique_ptr<ActionSpace<action_t>>&& action_space) :
	obs_space(std::move(obs_space)), action_space(std::move(action_space)) {}

template <typename O, typename A> scip::Model& Env<O, A>::model() noexcept {
	return solve_controller.model();
}

template <typename O, typename A>
auto Env<O, A>::_reset(scip::Model new_model) -> std::tuple<obs_t, bool> {
	solve_controller = internal::ReverseControl(std::move(new_model));
	solve_controller.wait();
	return {obs_space->get(model()), solve_controller.is_done()};
}

template <typename O, typename A>
auto Env<O, A>::_step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	auto const var = action_space->get(model(), action);
	solve_controller.resume(var);
	solve_controller.wait();
	return {obs_space->get(model()), 0., solve_controller.is_done(), info_t{}};
}

} // namespace branching
} // namespace ecole
