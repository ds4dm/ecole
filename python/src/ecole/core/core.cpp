#define FORCE_IMPORT_ARRAY

#include <limits>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/exception.hpp"
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
		.def("discard", &RandomEngine::discard, py::arg("n"), R"(
			Advance the internal state by n times.

			Equivalent to calling operator() n times and discarding the result.
		)")
		.def("__call__", &RandomEngine::operator(), R"(
			Generate a pseudo-random value.

			The state of the engine is advanced by one position.
		)")
		.def(py::self == py::self)   // NOLINT(misc-redundant-expression)  pybind specific syntax
		.def(py::self != py::self);  // NOLINT(misc-redundant-expression)  pybind specific syntax

	m.def("seed", &ecole::seed, py::arg("val"), "Seed the global source of randomness in Ecole.");
	m.def("spawn_random_engine", &ecole::spawn_random_engine, R"(
		Create new random engine deriving from global source of randomness.

		The global source of randomness is advance so two random engien created successively have different states.
	)");

	py::register_exception<ecole::Exception>(m, "Exception");

	scip::bind_submodule(m.def_submodule("scip"));
	observation::bind_submodule(m.def_submodule("observation"));
	reward::bind_submodule(m.def_submodule("reward"));
	dynamics::bind_submodule(m.def_submodule("dynamics"));
}
