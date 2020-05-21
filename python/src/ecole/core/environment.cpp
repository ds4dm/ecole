#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/configuring-dynamics.hpp"
#include "ecole/environment/exception.hpp"

#include "core.hpp"
#include "nonstd.hpp"

/******************************
 *  Customize PyBind Casting  *
 ******************************/

namespace pybind11 {
namespace detail {

using namespace ecole;

/**
 * Custom caster for @ref scip::Param.
 *
 * The default caster for variant greedily cast to the first compile-time compatible
 * type found in the variant.
 * However it is not necessarily the best one. For instance, given that
 * @ref scip::Param contains both `char` and `std::string`, the default caster cast all
 * Python `str` as char, and complains (dynamically) when the `str` is longer than one
 * character.
 * Here, we cast the python value to the largest possible container, knowing that
 * @ref scip::Model::set_param will be able to downcast based on the SCIP parameter
 * type.
 *
 * Implement a custom Python to C++ caster for scip::Param
 */
template <> struct type_caster<scip::Param> : variant_caster<scip::Param> {
public:
	/**
	 * Description and vlaue varaible.
	 *
	 * This macro establishes the name description in function signatures and declares a
	 * local variable `value` of type @ref scip::Param.
	 */
	PYBIND11_TYPE_CASTER(scip::Param, _("Union[bool, int, float, str]"));

	/**
	 * Conversion from Python to C++.
	 *
	 * Convert a PyObject into a @ref scip::Param instance or return false upon failure.
	 * The second argument indicates whether implicit conversions should be applied.
	 * Uses a variant with only the largest container, relying on
	 * @ref scip::Model::set_param to properly downcast when needed.
	 *
	 * @param src The PyObject to convert from.
	 */
	bool load(handle src, bool) {
		using ParamHelper = nonstd::variant<bool, scip::long_int, scip::real, std::string>;
		try {
			value = nonstd::visit(
				[](auto&& val) -> scip::Param { return val; }, src.cast<ParamHelper>());
			return true;
		} catch (...) {
			return false;
		}
	}
};

}  // namespace detail
}  // namespace pybind11

/********************
 *  Ecole Bindings  *
 ********************/

namespace ecole {
namespace environment {

namespace py = pybind11;

template <typename Dynamics> auto dynamics_class(py::module& m, char const* name) {
	return py::class_<Dynamics>(m, name)  //
		.def(
			"reset_dynamics",
			&Dynamics::reset_dynamics,
			py::arg("state"),
			py::call_guard<py::gil_scoped_release>())
		.def(
			"step_dynamics",
			&Dynamics::step_dynamics,
			py::arg("state"),
			py::arg("action"),
			py::call_guard<py::gil_scoped_release>());
}

void bind_submodule(pybind11::module m) {
	m.doc() = "Ecole collection of environments.";

	py::register_exception<Exception>(m, "Exception");

	py::class_<State>(m, "State")
		.def_readonly("model", &State::model)  //
		.def(py::init<scip::Model const&>());

	dynamics_class<BranchingDynamics>(m, "BranchingDynamics")  //
		.def(py::init<bool>(), py::arg("pseudo_candidates") = false);

	dynamics_class<ConfiguringDynamics>(m, "ConfiguringDynamics")  //
		.def(py::init<>());
}

}  // namespace environment
}  // namespace ecole
