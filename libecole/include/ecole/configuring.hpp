#pragma once

#include <memory>
#include <tuple>

#include "ecole/base.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace configuring {

template <typename Action> class ActionFunction {
public:
	using action_t = Action;

	virtual ~ActionFunction() = default;
	virtual void set(scip::Model& model, Action const& action) = 0;
	virtual std::unique_ptr<ActionFunction> clone() const = 0;
};

template <typename Action> class Configure : public ActionFunction<Action> {
public:
	using typename ActionFunction<Action>::action_t;

	std::string const param;

	Configure(std::string param) noexcept;
	void set(scip::Model& model, action_t const& action) override;
	virtual std::unique_ptr<ActionFunction<Action>> clone() const override;
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
		ptr<base::ObservationFunction<obs_t>>&& obs_func,
		ptr<ActionFunction<action_t>>&& action_func);

private:
	ptr<scip::Model> _model;
	ptr<base::ObservationFunction<obs_t>> obs_func;
	ptr<ActionFunction<action_t>> action_func;

	std::tuple<obs_t, bool> _reset(ptr<scip::Model>&& model) override;
	std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) override;
	bool is_done() const noexcept;
};

template <typename A>
Configure<A>::Configure(std::string param) noexcept : param(std::move(param)) {}

template <typename A> void Configure<A>::set(scip::Model& model, action_t const& action) {
	model.set_param(param, action);
}

template <typename A>
auto Configure<A>::clone() const -> std::unique_ptr<ActionFunction<A>> {
	return std::make_unique<Configure<A>>(*this);
}

template <typename A, typename O, template <typename...> class H>
Env<A, O, H>::Env(
	ptr<base::ObservationFunction<obs_t>>&& obs_func,
	ptr<ActionFunction<action_t>>&& action_func) :
	obs_func(std::move(obs_func)), action_func(std::move(action_func)) {}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::_reset(ptr<scip::Model>&& model) -> std::tuple<obs_t, bool> {
	_model = std::move(model);
	return {obs_func->get(*_model), is_done()};
}

template <typename A, typename O, template <typename...> class H>
auto Env<A, O, H>::_step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	action_func->set(*_model, action);
	_model->solve();
	return {obs_func->get(*_model), 0., true, info_t{}};
}

template <typename A, typename O, template <typename...> class H>
bool Env<A, O, H>::is_done() const noexcept {
	return _model->is_solved();
}

}  // namespace configuring
}  // namespace ecole
