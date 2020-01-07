#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(base, m) {
	m.doc() = "Abstract base classes for ecole environments.";

	py11::class_<py::EnvBase>(m, "Env")  //
		.def("seed", py11::overload_cast<>(&py::EnvBase::seed, py11::const_))
		.def("seed", py11::overload_cast<py::EnvBase::seed_t>(&py::EnvBase::seed))
		.def(
			"reset",
			[](py::EnvBase& env, scip::Model const& model) {
				return env.reset(scip::Model{model});
			})
		.def("reset", py11::overload_cast<std::string const&>(&py::EnvBase::reset))
		.def("step", [](py::EnvBase& env, py11::object action) {
			return env.step(py::Action<py11::object>(action));
		});
}
