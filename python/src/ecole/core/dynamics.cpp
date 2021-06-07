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

	dynamics_class<BranchingDynamics>{m, "BranchingDynamics"}
		.def_reset_dynamics()
		.def_step_dynamics()
		.def_set_dynamics_random_state()
		.def(py::init<bool>(), py::arg("pseudo_candidates") = false);

	using idx_t = typename BranchingGUBDynamics::Action::value_type;
	using array_t = py::array_t<idx_t, py::array::c_style | py::array::forcecast>;
	dynamics_class<BranchingGUBDynamics>{m, "BranchingGUBDynamics"}
		.def_reset_dynamics()
		.def_set_dynamics_random_state()
		.def(
			"step_dynamics",
			[](BranchingGUBDynamics& self, scip::Model& model, array_t const& action) {
				auto const vars = nonstd::span{action.data(), static_cast<std::size_t>(action.size())};
				auto const release = py::gil_scoped_release{};
				return self.step_dynamics(model, vars);
			},
			py::arg("model"),
			py::arg("action"))
		.def(py::init<>());

	dynamics_class<ConfiguringDynamics>{m, "ConfiguringDynamics"}
		.def_reset_dynamics()
		.def_step_dynamics()
		.def_set_dynamics_random_state()
		.def(py::init<>());
}

}  // namespace ecole::dynamics
