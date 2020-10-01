#define FORCE_IMPORT_ARRAY

#include <limits>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/random.hpp"
#include "ecole/version.hpp"

#include "core.hpp"

using namespace ecole;
namespace py = pybind11;

PYBIND11_MODULE(core, m) {
	m.doc() = R"str(
		Root module for binding Ecole library.

		All the bindings of Ecole are submodule of this module to enable some adjustment in
		the user interface.
	)str";

	xt::import_numpy();

	py::class_<VersionInfo>(m, "VersionInfo")  //
		.def_readwrite("major", &VersionInfo::major)
		.def_readwrite("minor", &VersionInfo::minor)
		.def_readwrite("patch", &VersionInfo::patch)
		.def_readwrite("git_revision", &VersionInfo::git_revision);

	m.def("get_build_version", &get_build_version);
	m.def("get_build_scip_version", &get_build_scip_version);

	py::class_<RandomEngine>(m, "RandomEngine")  //
		.def_property_readonly_static(
			"min_seed", [](py::object const& /* cls */) { return std::numeric_limits<RandomEngine::result_type>::min(); })
		.def_property_readonly_static(
			"max_seed", [](py::object const& /* cls */) { return std::numeric_limits<RandomEngine::result_type>::max(); })
		.def(
			py::init<RandomEngine::result_type>(),
			py::arg("value") = RandomEngine::default_seed,
			"Construct the pseudo-random number engine.")
		.def(
			"seed",
			[](RandomEngine& self, RandomEngine::result_type value) { self.seed(value); },
			py::arg("value") = RandomEngine::default_seed,
			"Reinitialize the internal state of the random-number engine using new seed "
			"value.")
		.def(py::self == py::self)   // NOLINT(misc-redundant-expression)  pybind specific syntax
		.def(py::self != py::self);  // NOLINT(misc-redundant-expression)  pybind specific syntax

	scip::bind_submodule(m.def_submodule("scip"));
	observation::bind_submodule(m.def_submodule("observation"));
	reward::bind_submodule(m.def_submodule("reward"));
	environment::bind_submodule(m.def_submodule("environment"));
}
