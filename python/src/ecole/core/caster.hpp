#pragma once

#include <variant>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

/**
 * Cutomize PyBind casting for some types.
 */

namespace pybind11::detail {

/**
 * Custom caster for  ecole::NoneType.
 *
 * Cast to `None` in Python and does not cast to C++.
 */
template <> struct type_caster<ecole::NoneType> : void_caster<ecole::NoneType> {};

/**
 * Custom caster for  scip::Param.
 *
 * The default caster for variant greedily cast to the first compile-time compatible
 * type found in the variant.
 * However it is not necessarily the best one. For instance, given that
 *  scip::Param contains both `char` and `std::string`, the default caster cast all
 * Python `str` as char, and complains (dynamically) when the `str` is longer than one
 * character.
 * Here, we cast the python value to the largest possible container, knowing that
 *  scip::Model::set_param will be able to downcast based on the SCIP parameter
 * type.
 *
 * Implement a custom Python to C++ caster for scip::Param
 */
template <> struct type_caster<ecole::scip::Param> : variant_caster<ecole::scip::Param> {
public:
	/**
	 * Description and value variable.
	 *
	 * This macro establishes the name description in function signatures and declares a
	 * local variable `value` of type  scip::Param.
	 */
	PYBIND11_TYPE_CASTER(ecole::scip::Param, _("Union[bool, int, float, str]"));  // NOLINT

	/**
	 * Conversion from Python to C++.
	 *
	 * Convert a PyObject into a  scip::Param instance or return false upon failure.
	 * The second argument indicates whether implicit conversions should be applied.
	 * Uses a variant with only the largest container, relying on
	 *  scip::Model::set_param to properly downcast when needed.
	 *
	 * @param src The PyObject to convert from.
	 */
	bool load(handle src, bool /*implicit_conversion*/) {
		using namespace ecole;
		using ParamHelper = std::variant<bool, SCIP_Longint, SCIP_Real, std::string>;

		try {
			value =
				std::visit([](auto&& val) -> scip::Param { return std::forward<decltype(val)>(val); }, src.cast<ParamHelper>());
			return true;
		} catch (...) {
			return false;
		}
	}

	/**
	 * Conversion from C++ to Python.
	 *
	 * Using the default variant caster from PyBind.
	 */
	using variant_caster::cast;
};

}  // namespace pybind11::detail
