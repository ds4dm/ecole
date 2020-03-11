#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/configuring.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "environment/adaptor.hpp"
#include "nonstd.hpp"
#include "observation/adaptor.hpp"

namespace py11 = pybind11;

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

PYBIND11_MODULE(environment, m) {
	m.doc() = "Ecole collection of environments.";

	// Import of abstract required for resolving inheritance to abstract base types
	py11::module const abstract_mod = py11::module::import("ecole.abstract");

	pyenvironment::env_class_<environment::Branching>(m, "Branching")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>())
		.def(py11::init<>([] {
			return pyenvironment::Env<environment::Branching>{
				std::make_shared<pyobservation::ObsFunction<observation::NodeBipartite>>(),
				std::make_shared<reward::IsDone>(),
				std::make_shared<termination::WhenSolved>()};
		}))
		.def_property(
			"state",
			&pyenvironment::Env<environment::Branching>::state,
			&pyenvironment::Env<environment::Branching>::state);
	;

	pyenvironment::env_class_<environment::Configuring>(m, "Configuring")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>())
		.def(py11::init<>([] {
			return pyenvironment::Env<environment::Configuring>{
				std::make_shared<pyobservation::ObsFunction<observation::NodeBipartite>>(),
				std::make_shared<reward::IsDone>(),
				std::make_shared<termination::WhenSolved>()};
		}))
		.def_property(
			"state",
			&pyenvironment::Env<environment::Configuring>::state,
			&pyenvironment::Env<environment::Configuring>::state);
	;
}
