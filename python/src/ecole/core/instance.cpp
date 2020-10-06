#include <memory>
#include <tuple>

#include <pybind11/pybind11.h>

#include "ecole/instance/capacitated-facility-location.hpp"
#include "ecole/instance/combinatorial-auction.hpp"
#include "ecole/instance/independent-set.hpp"
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

/**
 * Bind a string constructor for Enums.
 */
template <typename PyEnum> void def_init_str(PyEnum& py_enum);

void bind_submodule(py::module const& m) {
	m.doc() = "Random instance generators for Ecole.";

	// The Set Cover parameters used in constructor, generate_instance, and attributes
	auto constexpr set_cover_params = std::tuple{
		Member{"n_rows", &SetCoverGenerator::Parameters::n_rows},
		Member{"n_cols", &SetCoverGenerator::Parameters::n_cols},
		Member{"density", &SetCoverGenerator::Parameters::density},
		Member{"max_coef", &SetCoverGenerator::Parameters::max_coef},
	};
	// Bind SetCoverGenerator and remove intermediate Parameter class
	auto set_cover_gen = py::class_<SetCoverGenerator>{m, "SetCoverGenerator"};
	def_generate_instance(set_cover_gen, set_cover_params);
	def_init(set_cover_gen, set_cover_params);
	def_attributes(set_cover_gen, set_cover_params);
	def_iterator(set_cover_gen);
	set_cover_gen.def("seed", &SetCoverGenerator::seed, py::arg("seed"));

	// The Independent Set parameters used in constructor, generate_instance, and attributes
	auto constexpr independent_set_params = std::tuple{
		Member{"n_nodes", &IndependentSetGenerator::Parameters::n_nodes},
		Member{"edge_probability", &IndependentSetGenerator::Parameters::edge_probability},
		Member{"affinity", &IndependentSetGenerator::Parameters::affinity},
		Member{"graph_type", &IndependentSetGenerator::Parameters::graph_type},
	};
	// Create class for IndependenSetGenerator
	auto independent_set_gen = py::class_<IndependentSetGenerator>{m, "IndependentSetGenerator"};
	// Bind IndependenSetGenerator::Parameter::GraphType enum
	auto graph_type = py::enum_<IndependentSetGenerator::Parameters::GraphType>{independent_set_gen, "GraphType"};
	graph_type  //
		.value("barabasi_albert", IndependentSetGenerator::Parameters::GraphType::barabasi_albert)
		.value("erdos_renyi", IndependentSetGenerator::Parameters::GraphType::erdos_renyi)
		.export_values();
	// Add contructor from str to IndependenSetGenerator::Parameter::GraphType and make it implicit
	def_init_str(graph_type);
	// Bind IndependentSetGenerator methods and remove intermediate Parameter class
	def_generate_instance(independent_set_gen, independent_set_params);
	def_init(independent_set_gen, independent_set_params);
	def_attributes(independent_set_gen, independent_set_params);
	def_iterator(independent_set_gen);
	independent_set_gen.def("seed", &IndependentSetGenerator::seed, py::arg("seed"));

	// The Combinatorial Auction parameters used in constructor, generate_instance, and attributes
	auto constexpr combinatorial_auction_params = std::tuple{
		Member{"n_items", &CombinatorialAuctionGenerator::Parameters::n_items},
		Member{"n_bids", &CombinatorialAuctionGenerator::Parameters::n_bids},
		Member{"min_value", &CombinatorialAuctionGenerator::Parameters::min_value},
		Member{"max_value", &CombinatorialAuctionGenerator::Parameters::max_value},
		Member{"value_deviation", &CombinatorialAuctionGenerator::Parameters::value_deviation},
		Member{"add_item_prob", &CombinatorialAuctionGenerator::Parameters::add_item_prob},
		Member{"max_n_sub_bids", &CombinatorialAuctionGenerator::Parameters::max_n_sub_bids},
		Member{"additivity", &CombinatorialAuctionGenerator::Parameters::additivity},
		Member{"budget_factor", &CombinatorialAuctionGenerator::Parameters::budget_factor},
		Member{"resale_factor", &CombinatorialAuctionGenerator::Parameters::resale_factor},
		Member{"integers", &CombinatorialAuctionGenerator::Parameters::integers},
	};
	// Bind CombinatorialAuctionGenerator and remove intermediate Parameter class
	auto combinatorial_auction_gen = py::class_<CombinatorialAuctionGenerator>{m, "CombinatorialAuctionGenerator"};
	def_generate_instance(combinatorial_auction_gen, combinatorial_auction_params);
	def_init(combinatorial_auction_gen, combinatorial_auction_params);
	def_attributes(combinatorial_auction_gen, combinatorial_auction_params);
	def_iterator(combinatorial_auction_gen);
	combinatorial_auction_gen.def("seed", &CombinatorialAuctionGenerator::seed, py::arg("seed"));

	// The Capacitated Facility Location parameters used in constructor, generate_instance, and attributes
	auto constexpr capacitated_facility_location_params = std::tuple{
		Member{"n_customers", &CapacitatedFacilityLocationGenerator::Parameters::n_customers},
		Member{"n_facilities", &CapacitatedFacilityLocationGenerator::Parameters::n_facilities},
		Member{"ratio", &CapacitatedFacilityLocationGenerator::Parameters::ratio},
	};
	// Bind CapacitatedFacilityLocationGenerator and remove intermediate Parameter class
	auto capacitated_facility_location_gen =
		py::class_<CapacitatedFacilityLocationGenerator>{m, "CapacitatedFacilityLocationGenerator"};
	def_generate_instance(capacitated_facility_location_gen, capacitated_facility_location_params);
	def_init(capacitated_facility_location_gen, capacitated_facility_location_params);
	def_attributes(capacitated_facility_location_gen, capacitated_facility_location_params);
	def_iterator(capacitated_facility_location_gen);
	capacitated_facility_location_gen.def("seed", &CapacitatedFacilityLocationGenerator::seed, py::arg(" seed"));
}

/******************************************
 *  Binding code for instance generators  *
 ******************************************/

/**
 * Implementation of def_generate_instance to unpack tuple.
 */
template <typename PyClass, typename... Members>
void def_generate_instance_impl(PyClass& py_class, Members&&... members) {
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
}

template <typename PyClass, typename MemberTuple>
void def_generate_instance(PyClass& py_class, MemberTuple&& members_tuple) {
	// Forward call to impl in order to unpack the tuple
	std::apply(
		[&py_class](auto&&... members) {
			def_generate_instance_impl(py_class, std::forward<decltype(members)>(members)...);
		},
		std::forward<MemberTuple>(members_tuple));
}

/**
 * Implementation of def_init to unpack tuple.
 */
template <typename PyClass, typename... MemberTuple> void def_init_impl(PyClass& py_class, MemberTuple&&... members) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	using Parameters = typename Generator::Parameters;
	// Instantiate the C++ parameters at compile time to get default parameters.
	static constexpr auto default_params = Parameters{};
	// Bind a constructor that takes as input all parameters
	py_class.def(
		// Get the type of each parameter and add it to the Python constructor
		py::init([](RandomEngine const* random_engine, utility::return_t<decltype(members.value)>... params) {
			// Dispatch to the C++ constructors with a Parameter struct
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
}

template <typename PyClass, typename MemberTuple> void def_init(PyClass& py_class, MemberTuple&& members_tuple) {
	// Forward call to impl in order to unpack the tuple
	std::apply(
		[&py_class](auto&&... members) { def_init_impl(py_class, std::forward<decltype(members)>(members)...); },
		std::forward<MemberTuple>(members_tuple));
}

template <typename PyClass, typename MemberTuple> void def_attributes(PyClass& py_class, MemberTuple&& members_tuple) {
	// Lambda with core logic to unpack tuple in individual parameters
	auto def_attributes_impl = [&py_class](auto&&... members) {
		// The C++ class being wrapped
		using Generator = typename PyClass::type;
		// Bind attribute access for each member variable (comma operator fold expression).
		// Fetch the value on the Parameter object of the C++ class
		((py_class.def_property_readonly(
			 members.name, [value = members.value](Generator& self) { return std::invoke(value, self.get_parameters()); })),
		 ...);
	};

	// Forward call to lambda in order to unpack the tuple
	std::apply(def_attributes_impl, std::forward<MemberTuple>(members_tuple));
}

template <typename PyClass> void def_iterator(PyClass& py_class) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	py_class.def("__iter__", [](Generator& self) -> Generator& { return self; });
	py_class.def("__next__", &Generator::next);
}

template <typename PyEnum> void def_init_str(PyEnum& py_enum) {
	// The C++ being wrapped
	using Enum = typename PyEnum::type;
	// Add contructor from str to the enum and make it implicit
	py_enum.def(py::init(
		[members = py_enum.attr("__members__")](py::str const& name) { return members[name].template cast<Enum>(); }));
	py::implicitly_convertible<py::str, Enum>();
}

}  // namespace ecole::instance
