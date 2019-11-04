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

	virtual void set(scip::Model& model, Action const& action) = 0;
	virtual ~ActionSpace() = default;
};

template <typename ParamType> class Configure : public ActionSpace<ParamType> {
public:
	using typename ActionSpace<ParamType>::action_t;

	std::string const param;

	Configure(std::string param) noexcept;
	void set(scip::Model& model, action_t const& action) override;
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
