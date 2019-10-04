#include <pybind11/pybind11.h>

#include "ecole/learn2branch.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(ecole, m) {
	m.doc() = "Ecole library";

	m.def(
		"create_BranchEnv",
		[](std::string const& filename) { auto env = BranchEnv::from_file(filename); },
		"Temporary docstring.");
}
