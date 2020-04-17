#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/configuring.hpp"

#include "nonstd.hpp"

namespace py = pybind11;

using namespace ecole;

namespace pybind11 {
namespace detail {

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

template <typename Dynamics> auto dynamics_class(py::module& m, char const* name) {
	return py::class_<Dynamics>(m, name)  //
		.def(py::init<>())
		.def("reset_dynamics", &Dynamics::reset_dynamics, py::arg("state"))
		.def("step_dynamics", &Dynamics::step_dynamics, py::arg("state"), py::arg("action"));
}

PYBIND11_MODULE(environment, m) {
	m.doc() = "Ecole collection of environments.";

	py::register_exception<environment::Exception>(m, "Exception");

	dynamics_class<environment::BranchingDynamics>(m, "BranchingDynamics");

	dynamics_class<environment::ConfiguringDynamics>(m, "ConfiguringDynamics");
}
