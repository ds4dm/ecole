#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(base, m) {
	m.doc() = "Abstract base classes for ecole environments.";

	auto scip_module = py11::module::import("ecole.scip");

	py11::class_<py::ObsBase>(m, "Observation");
	py11::class_<py::ObsSpaceBase, std::shared_ptr<py::ObsSpaceBase>>(m, "ObservationSpace")
		.def("reset", &py::ObsSpaceBase::reset, py11::arg("model"))
		.def("get", &py::ObsSpaceBase::get, py11::arg("model"));

	py11::class_<base::RewardSpace, std::shared_ptr<base::RewardSpace>>(m, "RewardSpace")
		.def("reset", &base::RewardSpace::reset, py11::arg("model"))
		.def("get", &base::RewardSpace::get, py11::arg("model"), py11::arg("done") = false);

	py11::class_<base::TerminationSpace, std::shared_ptr<base::TerminationSpace>>(
		m, "TerminationSpace")
		.def("reset", &base::TerminationSpace::reset, py11::arg("model"))
		.def("is_done", &base::TerminationSpace::is_done, py11::arg("model"));

	py11::class_<py::EnvBase>(m, "Env")  //

		.def("seed", py11::overload_cast<>(&py::EnvBase::seed, py11::const_))
		.def(
			"seed",
			py11::overload_cast<py::EnvBase::seed_t>(&py::EnvBase::seed),
			py11::arg("value"))

		.def(
			"reset",
			[](py::EnvBase& env, scip::Model const& model) {
				return env.reset(scip::Model{model});
			},
			py11::arg("model"))
		.def(
			"reset",
			py11::overload_cast<std::string const&>(&py::EnvBase::reset),
			py11::arg("filename"))

		.def(
			"step",
			[](py::EnvBase& env, py11::object action) {
				return env.step(py::Action<py11::object>(action));
			},
			py11::arg("action"));
}
