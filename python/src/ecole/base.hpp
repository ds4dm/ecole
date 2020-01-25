#pragma once

#include <memory>

#include "ecole/base.hpp"

#include "base/observation.hpp"

namespace ecole {
namespace py {

namespace hidden {

struct Py_ActionBase {
	virtual ~Py_ActionBase() = default;
};

template <typename Action> struct Py_Action : public Py_ActionBase {
	// Using const ref because object is owned by python
	Action const& action;
	operator Action() const { return action; }
	Py_Action(Action const& action) : action(std::move(action)) {}
};

// Action function do not have a single base, but one per environment
template <template <typename Action> class ActionFunctionBase>
using Py_ActionFunctionBase = ActionFunctionBase<Py_ActionBase const&>;

template <typename ActionFunction, template <typename Action> class ActionFunctionBase>
struct Py_ActionFunction : public Py_ActionFunctionBase<ActionFunctionBase> {
	using py_action_t = Py_Action<typename ActionFunction::action_t>;
	using action_t = typename Py_ActionFunctionBase<ActionFunctionBase>::action_t;

	ActionFunction action_func;
	Py_ActionFunction(ActionFunction const& action_func) : action_func(action_func) {}
	Py_ActionFunction(ActionFunction&& action_func) : action_func(std::move(action_func)) {}
	template <typename... Args>
	Py_ActionFunction(Args... args) : action_func(std::forward<Args>(args)...) {}
};

template <typename AF, template <typename> class AFB, typename = void>
struct Py_ActionFunction_SFINAE;

template <typename> struct has { using type = void; };
template <typename T> using has_t = typename has<T>::type;

template <typename AF, template <typename> class AFB>
struct Py_ActionFunction_SFINAE<AF, AFB, has_t<decltype(&AF::set)>> :
	public Py_ActionFunction<AF, AFB> {
	using typename Py_ActionFunction<AF, AFB>::py_action_t;
	using typename Py_ActionFunction<AF, AFB>::action_t;

	using Py_ActionFunction<AF, AFB>::Py_ActionFunction;  // Inherit constructors

	// Core method to override
	void set(scip::Model& model, action_t const& action) override {
		this->action_func.set(model, dynamic_cast<py_action_t const&>(action));
	}

	// Clone could not go in a parent class because they are pure abstract
	std::unique_ptr<Py_ActionFunctionBase<AFB>> clone() const override {
		using this_t = typename std::remove_const_t<std::remove_pointer_t<decltype(this)>>;
		return std::make_unique<this_t>(this->action_func);
	}
};

// Specialization for get type of ActionFunction
template <typename AF, template <typename> class AFB>
struct Py_ActionFunction_SFINAE<AF, AFB, has_t<decltype(&AF::get)>> :
	public Py_ActionFunction<AF, AFB> {
	using typename Py_ActionFunction<AF, AFB>::py_action_t;
	using typename Py_ActionFunction<AF, AFB>::action_t;

	using Py_ActionFunction<AF, AFB>::Py_ActionFunction;  // Inherit constructors

	// Core method to override
	auto get(scip::Model& model, action_t const& action) -> decltype(
		this->action_func.get(model, std::declval<typename AF::action_t>())) override {
		return this->action_func.get(model, dynamic_cast<py_action_t const&>(action));
	}

	// Clone could not go in a parent class because they are pure abstract
	std::unique_ptr<Py_ActionFunctionBase<AFB>> clone() const override {
		using this_t = typename std::remove_const_t<std::remove_pointer_t<decltype(this)>>;
		return std::make_unique<this_t>(this->action_func);
	}
};

using Py_EnvBase =
	base::Env<Py_ActionBase const&, py::obs::ObsFunctionBase::obs_t, std::shared_ptr>;

template <  //
	template <typename, typename, template <typename...> class>
	class Env>
using Py_Env = Env<Py_EnvBase::action_t, Py_EnvBase::obs_t, std::shared_ptr>;

}  // namespace hidden

using ActionBase = hidden::Py_ActionBase;
template <typename A> using Action = hidden::Py_Action<A>;
template <template <typename> class AFB>
using ActionFunctionBase = hidden::Py_ActionFunctionBase<AFB>;
template <typename AF, template <typename> class AFB>
using ActionFunction = hidden::Py_ActionFunction_SFINAE<AF, AFB>;

using EnvBase = hidden::Py_EnvBase;
template <template <typename, typename, template <typename...> class> class E>
using Env = hidden::Py_Env<E>;

}  // namespace py
}  // namespace ecole
