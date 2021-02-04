#include <array>
#include <stdexcept>

#include <fmt/format.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include "ecole/instance/independent-set.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"

#include "instance/independent-set-graph.hpp"

namespace views = ranges::views;

namespace ecole::instance {

/*************************************
 *  IndependentSetGenerator methods  *
 *************************************/

IndependentSetGenerator::IndependentSetGenerator(Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{parameters_} {}
IndependentSetGenerator::IndependentSetGenerator(Parameters parameters_) :
	IndependentSetGenerator{parameters_, ecole::spawn_random_engine()} {}
IndependentSetGenerator::IndependentSetGenerator() : IndependentSetGenerator(Parameters{}) {}

scip::Model IndependentSetGenerator::next() {
	return generate_instance(parameters, random_engine);
}

void IndependentSetGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

/************************************************
 *  IndependentSetGenerator::generate_instance  *
 ************************************************/

namespace {

/** Make a graph according to the IndependentSetGenerator parameters specifications. */
auto make_graph(IndependentSetGenerator::Parameters parameters, RandomEngine& random_engine) -> Graph {
	switch (parameters.graph_type) {
	case IndependentSetGenerator::Parameters::GraphType::erdos_renyi:
		return Graph::erdos_renyi(parameters.n_nodes, parameters.edge_probability, random_engine);
	case IndependentSetGenerator::Parameters::GraphType::barabasi_albert:
		return Graph::barabasi_albert(parameters.n_nodes, parameters.affinity, random_engine);
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw std::invalid_argument{"Could not find graph type"};
	}
}

/** Create and add a single binary variable the whether the given node is part of the independent set.
 *
 * Variable are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured by
 * the scip*.
 * Their lifetime should not exceed that of the scip* (although that was already implied when creating them).
 */
auto add_node_var(SCIP* scip, Graph::Node n) -> SCIP_VAR* {
	auto const name = fmt::format("n_{}", n);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., 1.0, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Create and add all variables for the nodes. */
auto add_node_vars(SCIP* scip, std::size_t n_nodes) -> std::vector<SCIP_VAR*> {
	using Node = Graph::Node;
	return views::ints(Node{0}, Node{n_nodes}) | views::transform([scip](auto n) { return add_node_var(scip, n); }) |
				 ranges::to<std::vector>();
}

/** A class to reuse ressources between calls to `add_cons`. */
class ConstraintCreator {
public:
	using Node = Graph::Node;

	/** Allocate a buffer of variables used in create_cons_basic_linear. */
	ConstraintCreator(std::size_t buffer_size) : ones(buffer_size, 1.) { var_buffer.reserve(buffer_size); }

	/** Add constraint that at most one of the given variable can be in the independent set. */
	template <typename ConsContainer> void add_cons(SCIP* scip, ConsContainer const& vars) {
		auto const inf = SCIPinfinity(scip);
		auto const name = fmt::format("c_{}", idx++);
		auto cons = scip::create_cons_basic_linear(scip, name.c_str(), vars.size(), vars.data(), ones.data(), -inf, 1.0);
		scip::call(SCIPaddCons, scip, cons.get());
	}

	/** Variation of the where the internal buffer is used to store a temporary contiguous array of variables.
	 *
	 * This buffer is needed by the call to SCIPcreateConsBasicLinear.
	 */
	void add_cons(SCIP* scip, std::vector<SCIP_VAR*> const& node_vars, std::vector<Node> const& clique) {
		var_buffer.clear();
		for (auto node : clique) {
			var_buffer.push_back(node_vars[node]);
		}
		add_cons(scip, var_buffer);
	}

private:
	std::vector<SCIP_VAR*> var_buffer;
	std::vector<SCIP_Real> ones;
	std::size_t idx = 0;
};

/** A class to lookup fast if two nodes are in the same clique. */
class CliqueIndex {
public:
	using Node = Graph::Node;

	/** Build a constant time clique lookup. */
	CliqueIndex(std::vector<std::vector<Node>> const& clique_partition, std::size_t n_nodes) : cliques_ids(n_nodes) {
		for (auto&& [id, clique] : views::enumerate(clique_partition)) {
			for (auto node : clique) {
				cliques_ids[node] = id;
			}
		}
	}

	[[nodiscard]] auto are_in_same_clique(Node n1, Node n2) const noexcept -> bool {
		return cliques_ids[n1] == cliques_ids[n2];
	}

private:
	using CliqueId = std::size_t;
	std::vector<CliqueId> cliques_ids;
};

}  // namespace

scip::Model IndependentSetGenerator::generate_instance(Parameters parameters, RandomEngine& random_engine) {
	auto const graph = make_graph(parameters, random_engine);
	auto model = scip::Model::prob_basic();
	model.set_name(fmt::format("IndependentSet-{}", parameters.n_nodes));
	auto* const scip = model.get_scip_ptr();
	scip::call(SCIPsetObjsense, scip, SCIP_OBJSENSE_MAXIMIZE);

	auto vars = add_node_vars(scip, graph.n_nodes());
	auto cons_creator = ConstraintCreator{graph.n_nodes()};
	auto const clique_partition = graph.greedy_clique_partition();

	// Constraints for edges in clique are strenghen
	for (auto const& clique : clique_partition) {
		cons_creator.add_cons(scip, vars, clique);
	}

	// Constraints for other edges not in cliques
	auto clique_index = CliqueIndex{clique_partition, graph.n_nodes()};
	graph.edges_visit([&](auto edge) {
		auto [n1, n2] = edge;
		if (!clique_index.are_in_same_clique(n1, n2)) {
			cons_creator.add_cons(scip, std::array{vars[n1], vars[n2]});
		}
	});

	// Constraints for unconnected nodes otherwise SCIP complains
	for (auto node = Graph::Node{0}; node < graph.n_nodes(); ++node) {
		if (graph.degree(node) == 0) {
			cons_creator.add_cons(scip, std::array{vars[node]});
		}
	}

	return model;
}

}  // namespace ecole::instance
