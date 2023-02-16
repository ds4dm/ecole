#include <memory>
#include <tuple>

#include <pybind11/pybind11.h>

#include "ecole/instance/bin-packing.hpp"
#include "ecole/instance/capacitated-facility-location.hpp"
#include "ecole/instance/capacitated-vehicle-routing.hpp"
#include "ecole/instance/combinatorial-auction.hpp"
#include "ecole/instance/files.hpp"
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
template <typename PyClass, typename MemberTuple>
void def_generate_instance(PyClass& py_class, MemberTuple&& members_tuple, char const* docstring = "");

/**
 * Bind the constructor by unpacking the Parameter struct into individual function parameters.
 */
template <typename PyClass, typename MemberTuple>
void def_init(PyClass& py_class, MemberTuple&& members_tuple, char const* docstring = "");

/**
 * Bind all the parameters attributes by forwarding the call to get_paramters.
 */
template <typename PyClass, typename MemberTuple> void def_attributes(PyClass& py_class, MemberTuple&& members_tuple);

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

	// The sampling parameters used in constructor and attributes.
	auto constexpr file_params = std::tuple{
		Member{"directory", &FileGenerator::Parameters::directory},
		Member{"recursive", &FileGenerator::Parameters::recursive},
		Member{"sampling_mode", &FileGenerator::Parameters::sampling_mode},
	};
	// Bind FileGenerator and remove intermediate Parameter class
	auto file_gen = py::class_<FileGenerator>{m, "FileGenerator"};
	// Bind FileGenerator::Parameter::SamplingMode enum
	auto sampling_mode = py::enum_<FileGenerator::Parameters::SamplingMode>{file_gen, "SamplingMode", R"(
		A generator to iterate over files in a directory and load them into :py:class:<ecole.scip.Model>.
	)"};
	sampling_mode  //
		.value("replace", FileGenerator::Parameters::SamplingMode::replace)
		.value("remove", FileGenerator::Parameters::SamplingMode::remove)
		.value("remove_and_repeat", FileGenerator::Parameters::SamplingMode::remove_and_repeat);
	// Add contructor from str to FileGenerator::Parameter::SamplingMode and make it implicit
	def_init_str(sampling_mode);
	// Bind FileGenerator methods and remove intermediate Parameter class
	def_init(file_gen, file_params, R"(
		Create a generator to iterate over local problem files.

		Parameters
		--------
		directory:
			The path of the directory in which to look for files.
		recursive:
			Wether sub-directories are searched as well.
		sampling_mode:
			Method to iterate over files
				- "replace": Replace every file in the sampling pool right after it is sampled;
				- "remove": Remove every file from the sampling pool right after it is sampled and finish
					iteration when all files are sampled once;
				- "remove_and_repeat": Remove every file from the sampling pool right after it is sampled
					but repeat the procedure (with different order) after all files have been sampled.
	)");
	def_attributes(file_gen, file_params);
	def_iterator(file_gen);
	file_gen.def("seed", &FileGenerator::seed, py::arg(" seed"));

	// The Set Cover parameters used in constructor, generate_instance, and attributes
	auto constexpr set_cover_params = std::tuple{
		Member{"n_rows", &SetCoverGenerator::Parameters::n_rows},
		Member{"n_cols", &SetCoverGenerator::Parameters::n_cols},
		Member{"density", &SetCoverGenerator::Parameters::density},
		Member{"max_coef", &SetCoverGenerator::Parameters::max_coef},
	};
	// Bind SetCoverGenerator and remove intermediate Parameter class
	auto set_cover_gen = py::class_<SetCoverGenerator>{m, "SetCoverGenerator"};
	def_generate_instance(set_cover_gen, set_cover_params, R"(
		Generate a set cover MILP problem instance.

		Algorithm described in [Balas1980]_.

		Parameters
		----------
		n_rows:
			The number of rows.
		n_cols:
			The number of columns.
		density:
			The density of the constraint matrix.
			The value must be in the range ]0,1].
		max_coef:
			Maximum objective coefficient.
			The value must be greater than one.
		rng:
			The random number generator used to peform all sampling.

		References
		----------
			.. [Balas1980]
				Egon Balas and Andrew Ho.
				"Set covering algorithms using cutting planes, heuristics, and subgradient optimization: A computational study".
				*Mathematical Programming*, 12, pp. 37-60. 1980.
	)");
	def_init(set_cover_gen, set_cover_params);
	def_attributes(set_cover_gen, set_cover_params);
	def_iterator(set_cover_gen);
	set_cover_gen.def("seed", &SetCoverGenerator::seed, py::arg("seed"));

	// The Independent Set parameters used in constructor, generate_instance, and attributes
	auto constexpr independent_set_params = std::tuple{
		Member{"n_nodes", &IndependentSetGenerator::Parameters::n_nodes},
		Member{"graph_type", &IndependentSetGenerator::Parameters::graph_type},
		Member{"edge_probability", &IndependentSetGenerator::Parameters::edge_probability},
		Member{"affinity", &IndependentSetGenerator::Parameters::affinity},
	};
	// Create class for IndependenSetGenerator
	auto independent_set_gen = py::class_<IndependentSetGenerator>{m, "IndependentSetGenerator"};
	// Bind IndependenSetGenerator::Parameter::GraphType enum
	auto graph_type = py::enum_<IndependentSetGenerator::Parameters::GraphType>{independent_set_gen, "GraphType"};
	graph_type  //
		.value("barabasi_albert", IndependentSetGenerator::Parameters::GraphType::barabasi_albert)
		.value("erdos_renyi", IndependentSetGenerator::Parameters::GraphType::erdos_renyi)
		.export_values();  // TODO remove
	// Add contructor from str to IndependenSetGenerator::Parameter::GraphType and make it implicit
	def_init_str(graph_type);
	// Bind IndependentSetGenerator methods and remove intermediate Parameter class
	def_generate_instance(independent_set_gen, independent_set_params, R"(
		Generate an independent set MILP problem instance.

		Given an undireted graph, the problem is to find a maximum subset of nodes such that no pair of nodes are
		connected. There are one variable per node in the underlying graph. Instead of adding one constraint per edge, a
		greedy algorithm is run to replace these inequalities when clique is found. The maximization problem is
		unwheighted, that is all objective coefficients are equal to one.

		The problem are generated using the procedure from [Bergman2016]_, and the graphs are sampled following
		[Erdos1959]_ and [Barabasi1999]_.

		Parameters
		----------
		n_nodes:
			The number of nodes in the graph, and therefore of variable.
		graph_type:
			The method used in which to generate graphs.
			One of ``"barabasi_albert"`` or ``"erdos_renyi"``.
		edge_probability:
			The probability of generating each edge.
			This parameter must be in the range [0, 1].
			This parameter will only be used if ``graph_type == "erdos_renyi"``.
		affinity:
			The number of nodes each new node will be attached to, in the sampling scheme.
			This parameter must be an integer >= 1.
			This parameter will only be used if ``graph_type == "barabasi_albert"``.
		rng:
			The random number generator used to peform all sampling.

		References
		----------
			.. [Bergman2016]
				David Bergman, Andre A. Cire, Willem-Jan Van Hoeve, and John Hooker.
				"Decision diagrams for optimization", Section 4.6.4.
				*Springer International Publishing*, 2016.
			.. [Erdos1959]
				Paul Erdos and Alfréd Renyi.
				"On Random Graph"
				*Publicationes Mathematicae*, pp. 290-297, 1959.
			.. [Barabasi1999]
				Albert-László Barabási and Réka Albert.
				"Emergence of scaling in random networks"
				*Science* vol. 286, num. 5439, pp. 509-512, 1999.
	)");
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
		Member{"warnings", &CombinatorialAuctionGenerator::Parameters::warnings},
	};
	// Bind CombinatorialAuctionGenerator and remove intermediate Parameter class
	auto combinatorial_auction_gen = py::class_<CombinatorialAuctionGenerator>{m, "CombinatorialAuctionGenerator"};
	def_generate_instance(combinatorial_auction_gen, combinatorial_auction_params, R"(
		Generate a combinatorial auction MILP problem instance.

		This method generates an instance of a combinatorial auction problem based on the
		specified parameters and returns it as an ecole model.

		Algorithm described in [LeytonBrown2000]_.

		Parameters
		----------
		n_items:
			The number of items.
		n_bids:
			The number of bids.
		min_value:
			The minimum resale value for an item.
		max_value:
			The maximum resale value for an item.
		value_deviation:
			The deviation allowed for each bidder's private value of an item, relative from max_value.
		add_item_prob:
			The probability of adding a new item to an existing bundle.
			This parameters must be in the range [0,1].
		max_n_sub_bids:
			The maximum number of substitutable bids per bidder (+1 gives the maximum number of bids per bidder).
		additivity:
			Additivity parameter for bundle prices. Note that additivity < 0 gives sub-additive bids, while
			additivity > 0 gives super-additive bids.
		budget_factor:
			The budget factor for each bidder, relative to their initial bid's price.
		resale_factor:
			The resale factor for each bidder, relative to their initial bid's resale value.
		integers:
			Determines if the bid prices should be integral.
		warnings:
			Determines if warnings should be printed when invalid bundles are skipped in instance generation.
		rng:
			The random number generator used to peform all sampling.

		References
		----------
		.. [LeytonBrown2000]
			Kevin Leyton-Brown, Mark Pearson, and Yoav Shoham.
			"Towards a universal test suite for combinatorial auction algorithms".
			*Proceedings of ACM Conference on Electronic Commerce* (EC01) pp. 66-76.
			Section 4.3., the 'arbitrary' scheme. 2000.
	)");
	def_init(combinatorial_auction_gen, combinatorial_auction_params);
	def_attributes(combinatorial_auction_gen, combinatorial_auction_params);
	def_iterator(combinatorial_auction_gen);
	combinatorial_auction_gen.def("seed", &CombinatorialAuctionGenerator::seed, py::arg("seed"));

	// The Capacitated Facility Location parameters used in constructor, generate_instance, and attributes
	auto constexpr capacitated_facility_location_params = std::tuple{
		Member{"n_customers", &CapacitatedFacilityLocationGenerator::Parameters::n_customers},
		Member{"n_facilities", &CapacitatedFacilityLocationGenerator::Parameters::n_facilities},
		Member{"continuous_assignment", &CapacitatedFacilityLocationGenerator::Parameters::continuous_assignment},
		Member{"ratio", &CapacitatedFacilityLocationGenerator::Parameters::ratio},
		Member{"demand_interval", &CapacitatedFacilityLocationGenerator::Parameters::demand_interval},
		Member{"capacity_interval", &CapacitatedFacilityLocationGenerator::Parameters::capacity_interval},
		Member{"fixed_cost_cste_interval", &CapacitatedFacilityLocationGenerator::Parameters::fixed_cost_cste_interval},
		Member{"fixed_cost_scale_interval", &CapacitatedFacilityLocationGenerator::Parameters::fixed_cost_scale_interval},
	};
	// Bind CapacitatedFacilityLocationGenerator and remove intermediate Parameter class
	auto capacitated_facility_location_gen =
		py::class_<CapacitatedFacilityLocationGenerator>{m, "CapacitatedFacilityLocationGenerator"};
	def_generate_instance(capacitated_facility_location_gen, capacitated_facility_location_params, R"(
		Generate a capacitated facility location MILP problem instance.

		The capacitated facility location assigns a number of customers to be served from a number of facilities.
		Not all facilities need to be opened.
		In fact, the problem is to minimized the sum of the fixed costs for each facilities and the sum of transportation
		costs for serving a given customer from a given facility.
		In a variant of the problem, the customers can be served from multiple facilities and the associated variables
		become [0,1] continuous.

		The sampling algorithm is described in [Cornuejols1991]_, but uniform sampling as been replaced by *integer*
		uniform sampling.

		Parameters
		----------
		n_customers:
			The number of customers.
		n_facilities:
			The number of facilities.
		continuous_assignment:
			Whether variable for assigning a customer to a facility are binary or [0,1] continuous.
		ratio:
			After all sampling is performed, the capacities are scaled by `ratio * sum(demands) / sum(capacities)`.
		demand_interval:
			The customer demands are sampled independently as uniform integers in this interval [lower, upper[.
		capacity_interval:
			The facility capacities are sampled independently as uniform integers in this interval [lower, upper[.
		fixed_cost_cste_interval:
			The fixed costs are the sum of two terms.
			The first terms in the fixed costs for opening facilities are sampled independently as uniform integers
			in this interval [lower, upper[.
		fixed_cost_scale_interval:
			The fixed costs are the sum of two terms.
			The second terms in the fixed costs for opening facilities are sampled independently as uniform integers
			in this interval [lower, upper[ multiplied by the square root of their capacity prior to scaling.
			This second term reflects the economies of scale.
		rng:
			The random number generator used to peform all sampling.

		References
		----------
		.. [Cornuejols1991]
			Cornuejols G, Sridharan R, Thizy J-M.
			"A Comparison of Heuristics and Relaxations for the Capacitated Plant Location Problem".
			*European Journal of Operations Research* 50, pp. 280-297. 1991.
	)");
	def_init(capacitated_facility_location_gen, capacitated_facility_location_params);
	def_attributes(capacitated_facility_location_gen, capacitated_facility_location_params);
	def_iterator(capacitated_facility_location_gen);
	capacitated_facility_location_gen.def("seed", &CapacitatedFacilityLocationGenerator::seed, py::arg(" seed"));

	// The Capacitated Vehicle Routing parameters used in constructor, generate_instance, and attributes
	auto constexpr capacitated_vehicle_routing_params = std::tuple{
		Member{"filename", &CapacitatedVehicleRoutingLoader::Parameters::filename},
		Member{"n_vehicles", &CapacitatedVehicleRoutingLoader::Parameters::n_vehicles},
	};
	// Bind CapacitatedVehicleRoutingLoader and remove intermediate Parameter class
	auto capacitated_vehicle_routing_load =
		py::class_<CapacitatedVehicleRoutingLoader>{m, "CapacitatedVehicleRoutingLoader"};
	def_generate_instance(capacitated_vehicle_routing_load, capacitated_vehicle_routing_params, R"(
		Load a capacitated vehicle routing MILP problem instance.

		The capacitated vehicle routing problems assigns a number of vehicles to
		serve a number of customers. Not all vehicles need to be operate. 

		Parameters
		----------
    filename:
      The VRP file.
		n_vehicles:
			The number of vehicles.
	)");
	def_init(capacitated_vehicle_routing_load, capacitated_vehicle_routing_params);
	def_attributes(capacitated_vehicle_routing_load, capacitated_vehicle_routing_params);
	def_iterator(capacitated_vehicle_routing_load);
	capacitated_vehicle_routing_load.def("seed", &CapacitatedVehicleRoutingLoader::seed, py::arg(" seed"));

	// The Binpacking parameters used in constructor, generate_instance, and attributes
	auto constexpr binpacking_params = std::tuple{
		Member{"filename", &Binpacking::Parameters::filename},
		Member{"n_bins", &Binpacking::Parameters::n_bins},
	};
	// Bind Binpacking and remove intermediate Parameter class
	auto binpacking_load = py::class_<Binpacking>{m, "Binpacking"};
	def_generate_instance(binpacking_load, binpacking_params, R"(
		Load a Binpacking MILP problem instance.

    The Bin-packing Problem (BPP) can be described, using the terminology of knapsack problems, as follows. Given $n$ items and $m$ knapsacks (or bins), with $w_j$ = weight of each item j, $c$ = capacity of each bin. Assign each item to one bin so that the total weight doesn't exceed its capacity and the number of bins used is minimum.

    The same problem can be used to determine the number of minimum vehicles in Vehicle Routing Problem where bins represent vehicles and items represent customers demands.

		Parameters
		----------
    filename:
      The Binpacking problem file.
    n_bins:
      The number of bins available.
	)");
	def_init(binpacking_load, binpacking_params);
	def_attributes(binpacking_load, binpacking_params);
	def_iterator(binpacking_load);
	binpacking_load.def("seed", &Binpacking::seed, py::arg(" seed"));
}

/******************************************
 *  Binding code for instance generators  *
 ******************************************/

/**
 * Implementation of def_generate_instance to unpack tuple.
 */
template <typename PyClass, typename... Members>
void def_generate_instance_impl(PyClass& py_class, char const* docstring, Members&&... members) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	using Parameters = typename Generator::Parameters;
	// Instantiate the C++ parameters at compile time to get default parameters.
	// FIXME could be constexpr but GCC 7.3 (on conda) is complaining
	static auto const default_params = Parameters{};
	// Bind a the static method that takes as input all parameters
	py_class.def_static(
		"generate_instance",
		// Get the type of each parameter and add it to the Python function parameters
		[](utility::return_t<decltype(members.value)>... params, RandomGenerator& rng) {
			// Call the C++ static function with a Parameter struct
			return Generator::generate_instance(Parameters{params...}, rng);
		},
		// Set name for all function parameters.
		// Fetch default value on the default parameters
		(py::arg(members.name) = std::invoke(members.value, default_params))...,
		py::arg("rng"),
		py::call_guard<py::gil_scoped_release>(),
		docstring);
}

template <typename PyClass, typename MemberTuple>
void def_generate_instance(PyClass& py_class, MemberTuple&& members_tuple, char const* docstring) {
	// Forward call to impl in order to unpack the tuple
	std::apply(
		[&py_class, docstring](auto&&... members) {
			def_generate_instance_impl(py_class, docstring, std::forward<decltype(members)>(members)...);
		},
		std::forward<MemberTuple>(members_tuple));
}

/**
 * Implementation of def_init to unpack tuple.
 */
template <typename PyClass, typename... Members>
void def_init_impl(PyClass& py_class, char const* docstring, Members&&... members) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	using Parameters = typename Generator::Parameters;
	// Instantiate the C++ parameters at compile time to get default parameters.
	// FIXME could be constexpr but GCC 7.3 (on conda) is complaining
	static auto const default_params = Parameters{};
	// Bind a constructor that takes as input all parameters
	py_class.def(
		// Get the type of each parameter and add it to the Python constructor
		py::init([](utility::return_t<decltype(members.value)>... params, RandomGenerator const* rng) {
			// Dispatch to the C++ constructors with a Parameter struct
			if (rng == nullptr) {
				return std::make_unique<Generator>(Parameters{params...});
			}
			return std::make_unique<Generator>(Parameters{params...}, *rng);
		}),
		// Set name for all constructor parameters.
		// Fetch default value on the default parameters
		(py::arg(members.name) = std::invoke(members.value, default_params))...,
		// None as nullptr are allowed
		py::arg("rng").none(true) = py::none(),
		docstring);
}

template <typename PyClass, typename MemberTuple>
void def_init(PyClass& py_class, MemberTuple&& members_tuple, char const* docstring) {
	// Forward call to impl in order to unpack the tuple
	std::apply(
		[&](auto&&... members) { def_init_impl(py_class, docstring, std::forward<decltype(members)>(members)...); },
		std::forward<MemberTuple>(members_tuple));
}

template <typename PyClass, typename Member> void def_attributes_impl(PyClass& py_class, Member&& member) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	// Bind attribute access for a given member variable
	// Fetch the value on the Parameter object of the C++ class
	py_class.def_property_readonly(
		member.name, [value = member.value](Generator& self) { return std::invoke(value, self.get_parameters()); });
}

template <typename PyClass, typename MemberTuple> void def_attributes(PyClass& py_class, MemberTuple&& members_tuple) {
	// Forward call to impl in order to unpack the tuple
	std::apply(
		[&py_class](auto&&... members) {
			// Bind attribute access for each member variable (comma operator fold expression).
			((def_attributes_impl(py_class, std::forward<decltype(members)>(members))), ...);
		},
		std::forward<MemberTuple>(members_tuple));
}

template <typename PyClass> void def_iterator(PyClass& py_class) {
	// The C++ class being wrapped
	using Generator = typename PyClass::type;
	py_class.def("__iter__", [](Generator& self) -> Generator& { return self; });
	py_class.def("__next__", &Generator::next, py::call_guard<py::gil_scoped_release>());
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
