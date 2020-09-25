#pragma once

#include <pybind11/pybind11.h>

#include "ecole/utility/function-traits.hpp"

namespace ecole {

/**
 * Hold a class member variable function pointer and its name together.
 */
template <typename Ptr> struct Member {
	char const* name;
	Ptr data;

	constexpr Member(char const* name_, Ptr data_) : name(name_), data(data_) {}
};

/**
 * Bind plain data struct as a dataclass like Python class.
 *
 * Defines constructor with the struct member variables as parameter with their default values.
 * Defines Python atrributes for the member varaibles.
 */
template <typename PyClass, typename... Ptrs> auto def_data_class(PyClass& py_class, Member<Ptrs>... members) {
	// The C++ class being wrapped
	using Class = typename PyClass::type;
	// Instantiate the C++ class at compile time to get default parameters.
	static constexpr auto default_values = Class{};
	// Bind a constructor with one argument per C++ class member variable and set default value.
	py_class.def(
		pybind11::init<utility::return_t<Ptrs>...>(),
		(pybind11::arg(members.name) = std::invoke(members.data, default_values))...);
	// Bind attribute access for each member variable (comma operator fold expression).
	((py_class.def_readwrite(members.name, members.data)), ...);
	return py_class;
}

}  // namespace ecole
