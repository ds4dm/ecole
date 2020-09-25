#include <pybind11/pybind11.h>

#include "ecole/instance/set-cover.hpp"

#include "core.hpp"
#include "data-class.hpp"

namespace py = pybind11;

namespace ecole::instance {

void bind_submodule(py::module const& m) {
	m.doc() = "Random instance generators for Ecole.";

	auto set_cover_gen = py::class_<SetCoverGenerator>{m, "SetCoverGenerator"};

	auto set_cover_params = py::class_<SetCoverGenerator::Parameters>{set_cover_gen, "Parameters"};
	def_data_class(
		set_cover_params,
		Member{"n_rows", &SetCoverGenerator::Parameters::n_rows},
		Member{"n_cols", &SetCoverGenerator::Parameters::n_cols},
		Member{"density", &SetCoverGenerator::Parameters::density},
		Member{"max_coef", &SetCoverGenerator::Parameters::max_coef});

	set_cover_gen  //
		.def_static("generate_instance", &SetCoverGenerator::generate_instance)
		.def(py::init<>())
		.def(py::init<SetCoverGenerator::Parameters, RandomEngine>(), py::arg("parameters"), py::arg("random_engine"))
		.def("__iter__", [](SetCoverGenerator& self) -> SetCoverGenerator& { return self; })
		.def("__next__", &SetCoverGenerator::next);
}

}  // namespace ecole::instance
