#pragma once

#include <memory>

#include "ecole/base/environment.hpp"

namespace ecole {
namespace py {

namespace hidden {

struct Py_ObsBase {
	virtual ~Py_ObsBase() = default;
};

using Py_ObsSpaceBase = base::ObservationSpace<std::unique_ptr<Py_ObsBase>>;

template <typename Obs> struct Py_Obs : public Py_ObsBase {
	Obs obs;
	Py_Obs(Obs&& obs) : obs(std::move(obs)) {}
};

template <typename ObsSpace> struct Py_ObsSpace : public Py_ObsSpaceBase {
	using py_obs_t = Py_Obs<typename ObsSpace::obs_t>;
	using obs_t = typename Py_ObsSpaceBase::obs_t;

	ObsSpace obs_space;

	Py_ObsSpace(ObsSpace const& obs_space) : obs_space(obs_space) {}
	Py_ObsSpace(ObsSpace&& obs_space) : obs_space(std::move(obs_space)) {}
	template <typename... Args>
	Py_ObsSpace(Args... args) : obs_space(std::forward<Args>(args)...) {}

	obs_t get(scip::Model const& model) const override {
		return std::make_unique<py_obs_t>(obs_space.get(model));
	}
	std::unique_ptr<Py_ObsSpaceBase> clone() const override {
		return std::make_unique<Py_ObsSpace>(obs_space);
	}
};

struct Py_ActionBase {
	virtual ~Py_ActionBase() = default;
};

template <typename Action> struct Py_Action : public Py_ActionBase {
	// Using const ref because object is owned by python
	Action const& action;
	operator Action() const { return action; }
	Py_Action(Action const& action) : action(std::move(action)) {}
};

// Action space do not have a single base, but one per environment
template <template <typename Action> class ActionSpaceBase>
using Py_ActionSpaceBase = ActionSpaceBase<Py_ActionBase const&>;

template <typename ActionSpace, template <typename Action> class ActionSpaceBase>
struct Py_ActionSpace : public Py_ActionSpaceBase<ActionSpaceBase> {
	using py_action_t = Py_Action<typename ActionSpace::action_t>;
	using action_t = typename Py_ActionSpaceBase<ActionSpaceBase>::action_t;

	ActionSpace action_space;
	Py_ActionSpace(ActionSpace const& action_space) : action_space(action_space) {}
	Py_ActionSpace(ActionSpace&& action_space) : action_space(std::move(action_space)) {}
	template <typename... Args>
	Py_ActionSpace(Args... args) : action_space(std::forward<Args>(args)...) {}
};

template <typename AS, template <typename> class ASB, typename = void>
struct Py_ActionSpace_SFINAE;

template <typename> struct has { typedef void type; };
template <typename T> using has_t = typename has<T>::type;

template <typename AS, template <typename> class ASB>
struct Py_ActionSpace_SFINAE<AS, ASB, has_t<decltype(&AS::set)>> :
	public Py_ActionSpace<AS, ASB> {
	using typename Py_ActionSpace<AS, ASB>::py_action_t;
	using typename Py_ActionSpace<AS, ASB>::action_t;

	using Py_ActionSpace<AS, ASB>::Py_ActionSpace; // Inherit constructors

	// Core method to override
	void set(scip::Model& model, action_t const& action) override {
		this->action_space.set(model, dynamic_cast<py_action_t const&>(action));
	}

	// Clone could not go in a parent class because they are pure abstract
	std::unique_ptr<Py_ActionSpaceBase<ASB>> clone() const override {
		using this_t = typename std::remove_const_t<std::remove_pointer_t<decltype(this)>>;
		return std::make_unique<this_t>(this->action_space);
	}
};

// Specialization for get type of ActionSpace
template <typename AS, template <typename> class ASB>
struct Py_ActionSpace_SFINAE<AS, ASB, has_t<decltype(&AS::get)>> :
	public Py_ActionSpace<AS, ASB> {
	using typename Py_ActionSpace<AS, ASB>::py_action_t;
	using typename Py_ActionSpace<AS, ASB>::action_t;

	using Py_ActionSpace<AS, ASB>::Py_ActionSpace; // Inherit constructors

	// Core method to override
	auto get(scip::Model& model, action_t const& action) -> decltype(
		this->action_space.get(model, std::declval<typename AS::action_t>())) override {
		return this->action_space.get(model, dynamic_cast<py_action_t const&>(action));
	}

	// Clone could not go in a parent class because they are pure abstract
	std::unique_ptr<Py_ActionSpaceBase<ASB>> clone() const override {
		using this_t = typename std::remove_const_t<std::remove_pointer_t<decltype(this)>>;
		return std::make_unique<this_t>(this->action_space);
	}
};

using Py_EnvBase = base::Env<std::unique_ptr<Py_ObsBase>, Py_ActionBase const&>;

template <template <typename Obs, typename Action> class Env>
using Py_Env = Env<Py_EnvBase::obs_t, Py_EnvBase::action_t>;

} // namespace hidden

// Aliases for external use
using ObsBase = hidden::Py_ObsBase;
template <typename O> using Obs = hidden::Py_Obs<O>;
using ObsSpaceBase = hidden::Py_ObsSpaceBase;
template <typename OS> using ObsSpace = hidden::Py_ObsSpace<OS>;

using ActionBase = hidden::Py_ActionBase;
template <typename A> using Action = hidden::Py_Action<A>;
template <template <typename> class ASB>
using ActionSpaceBase = hidden::Py_ActionSpaceBase<ASB>;
template <typename AS, template <typename> class ASB>
using ActionSpace = hidden::Py_ActionSpace_SFINAE<AS, ASB>;

using EnvBase = hidden::Py_EnvBase;
template <template <typename, typename> class E> using Env = hidden::Py_Env<E>;

} // namespace py
} // namespace ecole
