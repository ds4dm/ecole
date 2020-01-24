#define FORCE_IMPORT_ARRAY
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

PYBIND11_MODULE(core, m) {
	m.doc() = R"str(
		Empty module used for multi-phase initialization of other module.

		All Ecole modules share a same shared library with multiple module initialization
		functions (PEP 489).
		This module serves as location for the shared library, as well as importing the
		numpy C API.
	)str";
	xt::import_numpy();
}
