#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "ecole/branching.hpp"
#include "ecole/observation.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Ecole library";

	using obs_t = std::unique_ptr<py::ObsBase>;
	using action_t = branching::Fractional::action_t;
	using Env = branching::Env<obs_t, action_t>;
	py11::class_<Env>(
		m, "Env") //
							// .def("reset", (std::tuple<Env::obs_t, bool>(Env::*)(std::string)) &
							// Env::reset) .def("step", &Env::step);
		.def("test", [](Env& env) { env.reset("helo"); });
	// .def(py11::init<
	//      std::unique_ptr<py::ObsSpaceBase>,
	//      std::unique_ptr<branching::ActionSpace<action_t>>>())
}
