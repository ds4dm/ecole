#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/dynamics/branching-gub.hpp"
#include "ecole/dynamics/branching.hpp"
#include "ecole/dynamics/configuring.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole::dynamics {

namespace py = pybind11;

template <typename Class, typename... ClassArgs> struct dynamics_class : public pybind11::class_<Class, ClassArgs...> {
	using pybind11::class_<Class, ClassArgs...>::class_;

	/** Bind reset_dynamics */
	template <typename... Args> auto def_reset_dynamics(Args&&... args) -> auto& {
		this->def(
			"reset_dynamics",
			&Class::reset_dynamics,
			py::arg("model"),
			py::call_guard<py::gil_scoped_release>(),
			std::forward<Args>(args)...);
		return *this;
	}

	/** Bind step_dynamics */
	template <typename... Args> auto def_step_dynamics(Args&&... args) -> auto& {
		this->def(
			"step_dynamics",
			&Class::step_dynamics,
			py::arg("model"),
			py::arg("action"),
			py::call_guard<py::gil_scoped_release>(),
			std::forward<Args>(args)...);
		return *this;
	}

	/** Bind set_dynamics_random_state */
	template <typename... Args> auto def_set_dynamics_random_state(Args&&... args) -> auto& {
		this->def(
			"set_dynamics_random_state",
			&Class::set_dynamics_random_state,
			py::arg("model"),
			py::arg("random_engine"),
			py::call_guard<py::gil_scoped_release>(),
			std::forward<Args>(args)...);
		return *this;
	}
};

void bind_submodule(pybind11::module_ const& m) {
	m.doc() = "Ecole collection of environment dynamics.";

	dynamics_class<BranchingDynamics>{m, "BranchingDynamics", R"(
		Single variable branching Dynamics.

		Based on a SCIP branching callback with maximal priority and no depth limit.
		The dynamics give the control back to the user every time the callback would be called.
		The user recieves as an action set the list of branching candidates, and is expected to select
		one of them as the action.
	)"}
		.def_reset_dynamics(R"(
			Start solving up to first branching node.

			Start solving with SCIP defaults (``SCIPsolve``) and give back control to the user on the
			first branching decision.
			Users can inherit from this dynamics to change the defaults settings such as presolving
			and cutting planes.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.

			Returns
			-------
				done:
					Whether the instance is solved.
					This can happen without branching, for instance if the instance is solved during presolving.
				action_set:
					List of branching candidates. Candidates depend on parameters in :py:meth:`__init__`.
		)")
		.def_step_dynamics(R"(
			Branch and resume solving until next branching.

			Branching is done on a single variable using ``SCIPbranchVar``.
			The control is given back to the user on the next branching decision or when done.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				action:
					The index the LP column of the variable to branch on. One element of the action set.

			Returns
			-------
				done:
					Whether the instance is solved.
				action_set:
					List of branching candidates. Candidates depend on parameters in :py:meth:`__init__`.
				)")
		.def_set_dynamics_random_state(R"(
			Set seeds on the :py:class:`~ecole.scip.Model`.

			Set seed parameters, including permutation, LP, and shift.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				random_engine:
					The source of randomness. Passed by the environment.
		)")
		.def(py::init<bool>(), py::arg("pseudo_candidates") = false, R"(
			Create new dynamics.

			Parameters
			----------
			pseudo_candidates:
				Whether the action set contains pseudo branching variable candidates (``SCIPgetPseudoBranchCands``)
				or LP branching variable candidates (``SCIPgetPseudoBranchCands``).
		)");

	using idx_t = typename BranchingGUBDynamics::Action::value_type;
	using array_t = py::array_t<idx_t, py::array::c_style | py::array::forcecast>;
	dynamics_class<BranchingGUBDynamics>{m, "BranchingGUBDynamics", R"(
		Global Upper Bound branching Dynamics.

		Based on a SCIP branching callback with maximal priority and no depth limit.
		The dynamics give the control back to the user every time the callback would be called.
		The user recieves as an action set the list of branching candidates, and is expected to select
		a subset of them to branch on their sum.

		.. warning::
			The function used to perform branching is provided by Ecole and has not been extensively tested on
			a large variety of problem instances.
	)"}
		.def_reset_dynamics(R"(
			Start solving up to first branching node.

			Start solving with SCIP defaults (``SCIPsolve``) and give back control to the user on the
			first branching decision.
			Users can inherit from this dynamics to change the defaults settings such as presolving
			and cutting planes.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.

			Returns
			-------
				done:
					Whether the instance is solved.
					This can happen without branching, for instance if the instance is solved during presolving.
				action_set:
					List of branching candidates (``SCIPgetPseudoBranchCands``).
		)")
		.def_set_dynamics_random_state(R"(
			Set seeds on the :py:class:`~ecole.scip.Model`.

			Set seed parameters, including permutation, LP, and shift.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				random_engine:
					The source of randomness. Passed by the environment.
		)")
		.def(
			"step_dynamics",
			[](BranchingGUBDynamics& self, scip::Model& model, array_t const& action) {
				auto const vars = nonstd::span{action.data(), static_cast<std::size_t>(action.size())};
				auto const release = py::gil_scoped_release{};
				return self.step_dynamics(model, vars);
			},
			py::arg("model"),
			py::arg("action"),
			R"(
			Branch and resume solving until next branching.

			Branching is done on the sum of the given variables using their LP or pseudo solution value.
			To make a valid branching, the sum of the LP or pseudo solution value of the given variables
			must be non integer.
			In the opposite case, an exception will be thrown,
			The control is given back to the user on the next branching decision or when done.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				action:
					A subset of of the variables of given in the action set.
					Not all subsets are valid (see above).

			Returns
			-------
				done:
					Whether the instance is solved.
				action_set:
					List of branching candidates (``SCIPgetPseudoBranchCands``).
		)")
		.def(py::init<>());

	dynamics_class<ConfiguringDynamics>{m, "ConfiguringDynamics", R"(
		Setting solving parameters dynamics.

		These dynamics are meant to be used as a (contextual) bandit to find good parameters for SCIP.
	)"}
		.def_reset_dynamics(R"(
			Does nothing.

			Users can inherit from this dynamics to change when in the solving process parameters will be set
			(for instance after presolving).

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.

			Returns
			-------
				done:
					Whether the instance is solved. Always false.
				action_set:
					Unused.
		)")
		.def_step_dynamics(R"(
			Set parameters and solve the instance.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				action:
					A mapping of parameter names and values.

			Returns
			-------
				done:
					Whether the instance is solved. Always true.
				action_set:
					Unused.
		)")
		.def_set_dynamics_random_state(R"(
			Set seeds on the :py:class:`~ecole.scip.Model`.

			Set seed parameters, including permutation, LP, and shift.

			Parameters
			----------
				model:
					The state of the Markov Decision Process. Passed by the environment.
				random_engine:
					The source of randomness. Passed by the environment.
		)")
		.def(py::init<>());
}

}  // namespace ecole::dynamics
