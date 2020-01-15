#include <pybind11/pybind11.h>

#include "ecole/termination.hpp"

namespace py11 = pybind11;

using namespace ecole;

namespace ecole {
namespace py {

template <typename C>
using term_space_class = py11::class_<C, base::TerminationSpace, std::shared_ptr<C>>;

}
}  // namespace ecole

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	auto base_module = py11::module::import("ecole.base");

	py::term_space_class<termination::Solved>(m, "Solved")  //
		.def(py11::init<>());
}
