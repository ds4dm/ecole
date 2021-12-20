#define FORCE_IMPORT_ARRAY

#include <limits>
#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/exception.hpp"
#include "ecole/random.hpp"

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

	py::class_<RandomGenerator>(m, "RandomGenerator")  //
		.def_property_readonly_static(
			"min_seed", [](py::object const& /* cls */) { return std::numeric_limits<RandomGenerator::result_type>::min(); })
		.def_property_readonly_static(
			"max_seed", [](py::object const& /* cls */) { return std::numeric_limits<RandomGenerator::result_type>::max(); })
		.def(
			py::init<RandomGenerator::result_type>(),
			py::arg("value") = RandomGenerator::default_seed,
			"Construct the pseudo-random number generator.")
		.def(
			"seed",
			[](RandomGenerator& self, RandomGenerator::result_type value) { self.seed(value); },
			py::arg("value") = RandomGenerator::default_seed,
			"Reinitialize the internal state of the random-number generator using new seed "
			"value.")
		.def("discard", &RandomGenerator::discard, py::arg("n"), R"(
			Advance the internal state by n times.

			Equivalent to calling operator() n times and discarding the result.
		)")
		.def("__call__", &RandomGenerator::operator(), R"(
			Generate a pseudo-random value.

			The state of the generator is advanced by one position.
		)")
		.def(py::self == py::self)  // NOLINT(misc-redundant-expression)  pybind specific syntax
		.def(py::self != py::self)  // NOLINT(misc-redundant-expression)  pybind specific syntax
		.def("__copy__", [](const RandomGenerator& self) { return std::make_unique<RandomGenerator>(self); })
		.def(
			"__deepcopy__",
			[](const RandomGenerator& self, py::dict const& /* memo */) { return std::make_unique<RandomGenerator>(self); },
			py::arg("memo"))
		.def(py::pickle(
			[](RandomGenerator const& self) { return serialize(self); },
			[](std::string const& data) { return std::make_unique<RandomGenerator>(deserialize(data)); }));

	m.def("seed", &ecole::seed, py::arg("val"), "Seed the global source of randomness in Ecole.");
	m.def("spawn_random_generator", &ecole::spawn_random_generator, R"(
		Create new random generator deriving from global source of randomness.

		The global source of randomness is advance so two random engien created successively have different states.
	)");

	py::register_exception<ecole::Exception>(m, "Exception");
	py::register_exception<ecole::IteratorExhausted>(m, "IteratorExhausted", PyExc_StopIteration);

	version::bind_submodule(m.def_submodule("version"));
	scip::bind_submodule(m.def_submodule("scip"));
	instance::bind_submodule(m.def_submodule("instance"));
	data::bind_submodule(m.def_submodule("data"));
	observation::bind_submodule(m.def_submodule("observation"));
	reward::bind_submodule(m.def_submodule("reward"));
	information::bind_submodule(m.def_submodule("information"));
	dynamics::bind_submodule(m.def_submodule("dynamics"));
}
