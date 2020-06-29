#define FORCE_IMPORT_ARRAY
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/version.hpp"

#include "core.hpp"

PYBIND11_MODULE(core, m) {
	m.doc() = R"str(
		Root module for binding Ecole library.

		All the bindings of Ecole are submodule of this module to enable some adjustment in
		the user interface.
	)str";

	xt::import_numpy();

	pybind11::class_<ecole::VersionInfo>(m, "VersionInfo")  //
		.def_readwrite("major", &ecole::VersionInfo::major)
		.def_readwrite("minor", &ecole::VersionInfo::minor)
		.def_readwrite("patch", &ecole::VersionInfo::patch)
		.def_readwrite("git_revision", &ecole::VersionInfo::git_revision);

	m.def("get_build_version", &ecole::get_build_version);
	m.def("get_build_scip_version", &ecole::get_build_scip_version);

	ecole::scip::bind_submodule(m.def_submodule("scip"));
	ecole::observation::bind_submodule(m.def_submodule("observation"));
	ecole::reward::bind_submodule(m.def_submodule("reward"));
	ecole::environment::bind_submodule(m.def_submodule("environment"));
}
