#include <memory>
#include <tuple>

#include <pybind11/pybind11.h>

#include "ecole/instance/set-cover.hpp"
#include "ecole/utility/function-traits.hpp"

#include "core.hpp"

namespace py = pybind11;

namespace ecole::instance {

/**
 * Hold a class member variable function pointer and its name together.
 */
template <typename Ptr> struct Member {
	char const* name;
	Ptr value;

	constexpr Member(char const* name_, Ptr value_) : name(name_), value(value_) {}
};

/**
 * Bind the static method `generate_instance` by unpacking the Parameter struct into individual function parameters.
 */
template <typename PyClass, typename Members> void def_generate_instance(PyClass& py_class, Members&& members_tuple);

/**
 * Bind the constructor by unpacking the Parameter struct into individual function parameters.
 */
template <typename PyClass, typename Members> void def_init(PyClass& py_class, Members&& members_tuple);

/**
 * Bind all the parameters attributes by forwarding the call to get_paramters.
 */
template <typename PyClass, typename Members> void def_attributes(PyClass& py_class, Members&& members_tuple);

/**
 * Bind infinite Python iteration using the `next` method.
 */
template <typename PyClass> void def_iterator(PyClass& py_class);

void bind_submodule(py::module const& m) {
	m.doc() = "Random instance generators for Ecole.";

	auto constexpr set_cover_params = std::tuple{
		Member{"n_rows", &SetCoverGenerator::Parameters::n_rows},
		Member{"n_cols", &SetCoverGenerator::Parameters::n_cols},
		Member{"density", &SetCoverGenerator::Parameters::density},
		Member{"max_coef", &SetCoverGenerator::Parameters::max_coef},
	};
	auto set_cover_gen = py::class_<SetCoverGenerator>{m, "SetCoverGenerator"};
	def_generate_instance(set_cover_gen, set_cover_params);
	def_init(set_cover_gen, set_cover_params);
	def_attributes(set_cover_gen, set_cover_params);
	def_iterator(set_cover_gen);
}

/******************************************
 *  Binding code for instance generators  *
 ******************************************/

template <typename PyClass, typename Members> void def_generate_instance(PyClass& py_class, Members&& members_tuple) {
	// Lambda with core logic to unpack tuple in individual parameters
	auto def_generate_instance_impl = [&py_class](auto... members) {
		// The C++ class being wrapped
		using Generator = typename PyClass::type;
		using Parameters = typename Generator::Parameters;
		// Instantiate the C++ parameters at compile time to get default parameters.
		static constexpr auto default_params = Parameters{};
		// Bind a the static method that takes as input all parameters
		py_class.def_static(
			"generate_instance",
			// Get the type of each parameter and add it to the Python function parameters
			[](RandomEngine& random_engine, utility::return_t<decltype(members.value)>... params) {
				// Call the C++ static function with a Parameter struct
				return Generator::generate_instance(random_engine, Parameters{params...});
			},
			py::arg("random_engine"),
			// Set name for all function parameters.
			// Fetch default value on the default parameters
			(py::arg(members.name) = std::invoke(members.value, default_params))...);
	};

	// Forward call to lambda in order to unpack the tuple
	std::apply(def_generate_instance_impl, std::forward<Members>(members_tuple));
}

template <typename PyClass, typename Members> void def_init(PyClass& py_class, Members&& members_tuple) {
	// Lambda with core logic to unpack tuple in individual parameters
	auto def_init_impl = [&py_class](auto... members) {
		// The C++ class being wrapped
		using Generator = typename PyClass::type;
		using Parameters = typename Generator::Parameters;
		// Instantiate the C++ parameters at compile time to get default parameters.
		static constexpr auto default_params = Parameters{};
		// Bind a constructor that takes as input all parameters
		py_class.def(
			// Get the type of each parameter and add it to the Python constructor
			py::init([](RandomEngine const* random_engine, utility::return_t<decltype(members.value)>... params) {
				// Disaptch to the C++ constructors with a Parameter struct
				if (random_engine == nullptr) {
					return std::make_unique<Generator>(Parameters{params...});
				}
				return std::make_unique<Generator>(*random_engine, Parameters{params...});
			}),
			// None as nullptr are allowed
			py::arg("random_engine").none(true) = py::none(),
			// Set name for all constructor parameters.
			// Fetch default value on the default parameters
			(py::arg(members.name) = std::invoke(members.value, default_params))...);
	};

	// Forward call to lambda in order to unpack the tuple
	std::apply(def_init_impl, std::forward<Members>(members_tuple));
}

template <typename PyClass, typename Members> void def_attributes(PyClass& py_class, Members&& members_tuple) {
	// Lambda with core logic to unpack tuple in individual parameters
	auto def_attributes_impl = [&py_class](auto... members) {
		// The C++ class being wrapped
		using Generator = typename PyClass::type;
		// Bind attribute access for each member variable (comma operator fold expression).
		// Fetch the value on the Parameter object of the C++ class
		((py_class.def_property_readonly(
			 members.name, [value = members.value](Generator& self) { return std::invoke(value, self.get_parameters()); })),
		 ...);
	};

	// Forward call to lambda in order to unpack the tuple
	std::apply(def_attributes_impl, std::forward<Members>(members_tuple));
}

template <typename PyClass> void def_iterator(PyClass& py_class) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	py_class.def("__iter__", [](Generator& self) -> Generator& { return self; });
	py_class.def("__next__", &Generator::next);
}

}  // namespace ecole::instance
