#pragma once

#include <memory>
#include <tuple>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace configuring {

template <typename Action> class ActionSpace {
public:
	using action_t = Action;

	virtual ~ActionSpace() = default;
	virtual void set(scip::Model& model, Action const& action) = 0;
	virtual std::unique_ptr<ActionSpace> clone() const = 0;
};

template <typename Action> class Configure : public ActionSpace<Action> {
public:
	using typename ActionSpace<Action>::action_t;

	std::string const param;

	Configure(std::string param) noexcept;
	void set(scip::Model& model, action_t const& action) override;
	virtual std::unique_ptr<ActionSpace<Action>> clone() const override;
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
		std::unique_ptr<base::ObservationSpace<obs_t>> obs_space,
		std::unique_ptr<ActionSpace<action_t>> action_space);

private:
	scip::Model _model;
	std::unique_ptr<base::ObservationSpace<obs_t>> obs_space;
	std::unique_ptr<ActionSpace<action_t>> action_space;

	std::tuple<obs_t, bool> _reset(scip::Model model) override;
	std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) override;
	bool is_done() const noexcept;
};

template <typename A>
Configure<A>::Configure(std::string param) noexcept : param(std::move(param)) {}

template <typename A> void Configure<A>::set(scip::Model& model, action_t const& action) {
	model.set_param(param, action);
}

template <typename A>
auto Configure<A>::clone() const -> std::unique_ptr<ActionSpace<A>> {
	return std::make_unique<Configure<A>>(*this);
}

template <typename O, typename A>
Env<O, A>::Env(
	std::unique_ptr<base::ObservationSpace<obs_t>> obs_space,
	std::unique_ptr<ActionSpace<action_t>> action_space) :
	obs_space(std::move(obs_space)), action_space(std::move(action_space)) {}

template <typename O, typename A>
auto Env<O, A>::_reset(scip::Model model) -> std::tuple<obs_t, bool> {
	_model = std::move(model);
	return {obs_space->get(_model), is_done()};
}

template <typename O, typename A>
auto Env<O, A>::_step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	action_space->set(_model, action);
	_model.solve();
	return {obs_space->get(_model), 0., true, {}};
}

template <typename O, typename A> bool Env<O, A>::is_done() const noexcept {
	return _model.is_solved();
}

}  // namespace configuring
}  // namespace ecole
