#define FORCE_IMPORT_ARRAY
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "core.hpp"

PYBIND11_MODULE(core, m) {
	m.doc() = R"str(
		Root module for binding Ecole library.

		All the bindings of Ecole are submodule of this module to enable some adjustment in
		the user interface.
	)str";

	xt::import_numpy();

	ecole::scip::bind_submodule(m.def_submodule("scip"));
	ecole::observation::bind_submodule(m.def_submodule("observation"));
	ecole::reward::bind_submodule(m.def_submodule("reward"));
	ecole::environment::bind_submodule(m.def_submodule("environment"));
}
