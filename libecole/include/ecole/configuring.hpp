#pragma once

#include <memory>
#include <tuple>

#include "ecole/environment/abstract.hpp"
#include "ecole/observation/abstract.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/termination/abstract.hpp"

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
	std::string const param;

	Configure(std::string param) noexcept;
	void set(scip::Model& model, Action const& action) override;
	virtual std::unique_ptr<ActionFunction<Action>> clone() const override;
};

template <
	typename Action,
	typename Observation,
	template <typename...> class Holder = std::unique_ptr>
class Environment : public environment::Environment<Action, Observation, Holder> {
public:
	using env_t = environment::Environment<Action, Observation, Holder>;
	using typename env_t::info_t;
	using typename env_t::seed_t;

	template <typename T> using ptr = typename env_t::template ptr<T>;

	Environment(
		ptr<observation::ObservationFunction<Observation>>&& obs_func,
		ptr<ActionFunction<Action>>&& action_func);

private:
	ptr<scip::Model> _model;
	ptr<observation::ObservationFunction<Observation>> obs_func;
	ptr<ActionFunction<Action>> action_func;

	std::tuple<Observation, bool> _reset(ptr<scip::Model>&& model) override;
	std::tuple<Observation, Reward, bool, info_t> _step(Action action) override;
	bool is_done() const noexcept;
};

/***********************************
 *  Implementation of Environment  *
 ***********************************/

template <typename A>
Configure<A>::Configure(std::string param) noexcept : param(std::move(param)) {}

template <typename A> void Configure<A>::set(scip::Model& model, A const& action) {
	model.set_param(param, action);
}

template <typename A>
auto Configure<A>::clone() const -> std::unique_ptr<ActionFunction<A>> {
	return std::make_unique<Configure<A>>(*this);
}

template <typename A, typename O, template <typename...> class H>
Environment<A, O, H>::Environment(
	ptr<observation::ObservationFunction<O>>&& obs_func,
	ptr<ActionFunction<A>>&& action_func) :
	obs_func(std::move(obs_func)), action_func(std::move(action_func)) {}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::_reset(ptr<scip::Model>&& model) -> std::tuple<O, bool> {
	_model = std::move(model);
	return {obs_func->get(*_model), is_done()};
}

template <typename A, typename O, template <typename...> class H>
auto Environment<A, O, H>::_step(A action) -> std::tuple<O, Reward, bool, info_t> {
	action_func->set(*_model, action);
	_model->solve();
	return {obs_func->get(*_model), 0., true, info_t{}};
}

template <typename A, typename O, template <typename...> class H>
bool Environment<A, O, H>::is_done() const noexcept {
	return _model->is_solved();
}

}  // namespace configuring
}  // namespace ecole
