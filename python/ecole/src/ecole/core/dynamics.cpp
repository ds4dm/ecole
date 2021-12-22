#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/dynamics/configuring.hpp"
#include "ecole/dynamics/primal-search.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole::dynamics {

namespace py = pybind11;
template <typename T> using Numpy = py::array_t<T, py::array::c_style | py::array::forcecast>;

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
			py::arg("rng"),
			py::call_guard<py::gil_scoped_release>(),
			std::forward<Args>(args)...);
		return *this;
	}
};

void bind_submodule(pybind11::module_ const& m) {
	m.doc() = "Ecole collection of environment dynamics.";

	{
		dynamics_class<BranchingDynamics>{m, "BranchingDynamics", R"(
			Single variable branching Dynamics.

			Based on a SCIP `branching callback <https://www.scipopt.org/doc/html/BRANCH.php>`_
			with maximal priority and no depth limit.
			The dynamics give the control back to the user every time the callback would be called.
			The user receives as an action set the list of branching candidates, and is expected to select
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
						List of indices of branching candidate variables.
						Available candidates depend on parameters in :py:meth:`__init__`.
						Variable indices (values in the ``action_set``) are their position in the original problem
						(``SCIPvarGetProbindex``).
						Variable ordering in the ``action_set`` is arbitrary.
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
						If an explicit ``ecole.Default`` is passed, then default SCIP branching is used, that is, the next
						branching rule is used fetch by SCIP according to their priorities.

				Returns
				-------
					done:
						Whether the instance is solved.
					action_set:
						List of indices of branching candidate variables.
						Available candidates depend on parameters in :py:meth:`__init__`.
						Variable indices (values in the ``action_set``) are their position in the original problem
						(``SCIPvarGetProbindex``).
						Variables ordering in the ``action_set`` is arbitrary.
					)")
			.def_set_dynamics_random_state(R"(
				Set seeds on the :py:class:`~ecole.scip.Model`.

				Set seed parameters, including permutation, LP, and shift.

				Parameters
				----------
					model:
						The state of the Markov Decision Process. Passed by the environment.
					rng:
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
	}

	{
		dynamics_class<ConfiguringDynamics>{m, "ConfiguringDynamics", R"(
			Setting solving parameters Dynamics.

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
					rng:
						The source of randomness. Passed by the environment.
			)")
			.def(py::init<>());
	}

	{
		using idx_t = typename PrimalSearchDynamics::Action::first_type::value_type;
		using val_t = typename PrimalSearchDynamics::Action::second_type::value_type;
		dynamics_class<PrimalSearchDynamics>{m, "PrimalSearchDynamics", R"(
			Search for primal solutions Dynamics.

			Based on a SCIP `primal heuristic <https://www.scipopt.org/doc/html/HEUR.php>`_
			callback with maximal priority, which executes
			after the processing of a node is finished (``SCIP_HEURTIMING_AFTERNODE``).
			The dynamics give the control back to the user a few times (trials) each time
			the callback is called. The agent receives as an action set the list of all non-fixed
			discrete variables at the current node (pseudo branching candidates), and is
			expected to give back as an action a partial primal solution, i.e., a value
			assignment for a subset of these variables.

		)"}
			.def_set_dynamics_random_state(R"(
				Set seeds on the :py:class:`~ecole.scip.Model`.

				Set seed parameters, including permutation, LP, and shift.

				Parameters
				----------
					model:
						The state of the Markov Decision Process. Passed by the environment.
					rng:
						The source of randomness. Passed by the environment.
			)")
			.def_reset_dynamics(R"(
				Start solving up to first primal heuristic call.

				Start solving with SCIP defaults (``SCIPsolve``) and give back control to the user on the
				first heuristic call.
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
						This can happen before the heuristic gets called, for instance if the instance is solved during presolving.
					action_set:
						List of non-fixed discrete variables (``SCIPgetPseudoBranchCands``).
			)")
			.def(
				"step_dynamics",
				[](PrimalSearchDynamics& self, scip::Model& model, std::pair<Numpy<idx_t>, Numpy<val_t>> const& action) {
					auto const indices = nonstd::span{action.first.data(), static_cast<std::size_t>(action.first.size())};
					auto const values = nonstd::span{action.second.data(), static_cast<std::size_t>(action.second.size())};
					auto const release = py::gil_scoped_release{};
					return self.step_dynamics(model, {indices, values});
				},
				py::arg("model"),
				py::arg("action"),
				R"(
				Try to obtain a feasible primal solution from the given (partial) primal solution.

				If the number of search trials per node is exceeded, then continue solving until
				the next time the heuristic gets called.

				To obtain a complete feasible solution, variables are fixed to their partial assignment
				values, and the rest of the variable assigments is deduced by solving an LP in probing
				mode. If the provided partial assigment is empty, then nothing is done.

				Parameters
				----------
					model:
						The state of the Markov Decision Process. Passed by the environment.
					action:
						A subset of the variables given in the action set, and their assigned values.

				Returns
				-------
					done:
						Whether the instance is solved.
					action_set:
						List of non-fixed discrete variables (``SCIPgetPseudoBranchCands``).
			)")
			.def(
				py::init<int, int, int, int>(),
				py::arg("trials_per_node") = 1,
				py::arg("depth_freq") = 1,
				py::arg("depth_start") = 0,
				py::arg("depth_stop") = -1,
				R"(
					Initialize new PrimalSearchDynamics.

					Parameters
					----------
						trials_per_node:
							Number of primal searches performed at each node (or -1 for an infinite number of trials).
						depth_freq:
							Depth frequency of when the primal search is called (``HEUR_FREQ`` in SCIP).
						depth_start:
							Tree depth at which the primal search starts being called (``HEUR_FREQOFS`` in SCIP).
						depth_stop:
							Tree depth after which the primal search stops being called (``HEUR_MAXDEPTH`` in SCIP).
				)");
	}
}

}  // namespace ecole::dynamics
