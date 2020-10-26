#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include "ecole/instance/independent-set.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/var.hpp"
#include "ecole/utility/random.hpp"

namespace views = ranges::views;

namespace ecole::instance {

/*************************************
 *  IndependentSetGenerator methods  *
 *************************************/

IndependentSetGenerator::IndependentSetGenerator(RandomEngine random_engine_, Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
IndependentSetGenerator::IndependentSetGenerator(Parameters parameters_) :
	IndependentSetGenerator{ecole::spawn_random_engine(), parameters_} {}
IndependentSetGenerator::IndependentSetGenerator() : IndependentSetGenerator(Parameters{}) {}

scip::Model IndependentSetGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void IndependentSetGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

/************************************************
 *  IndependentSetGenerator::generate_instance  *
 ************************************************/

namespace {

/** A simple symetric graph based on adjacency lists.
 */
class Graph {
public:
	using Node = std::size_t;
	using Edge = std::pair<Node, Node>;

	static auto erdos_renyi(std::size_t n_nodes, double edge_probability, RandomEngine& random_engine) -> Graph;
	static auto barabasi_albert(std::size_t n_nodes, std::size_t affinity, RandomEngine& random_engine) -> Graph;

	Graph(std::size_t n_nodes) : edges{n_nodes} {}

	/** Reserve size for each adjacency list. */
	void reserve(std::size_t degree);

	[[nodiscard]] auto n_nodes() const noexcept -> std::size_t { return edges.size(); }
	[[nodiscard]] auto degree(Node n) const noexcept -> std::size_t { return edges[n].size(); }
	[[nodiscard]] auto neighbors(Node n) const noexcept -> std::vector<Node> const& { return edges[n]; }
	[[nodiscard]] auto are_connected(Node popular, Node unpopular) const -> bool;

	/** Apply a function on all edges in the graph. */
	template <typename Func> void edges_visit(Func&& func) const;

	void add_edge(Edge edge);

	/** Partition the nodes in clique using greedy algorithm.
	 *
	 * @return Vector of cliques, each being a vector of nodes.
	 */
	[[nodiscard]] auto greedy_clique_partition() const -> std::vector<std::vector<Node>>;

private:
	// Vector likely more performant than list on small-sized small-count data due to more predictable cache usage
	using AdjacencyLists = std::vector<std::vector<Node>>;

	AdjacencyLists edges;

	Graph(AdjacencyLists edges_) : edges(std::move(edges_)) {}
};

auto Graph::are_connected(Node popular, Node unpopular) const -> bool {
	// We search in neighborhood of unpopular node rather than popular to reduce compexity of linear scan
	return std::find(neighbors(unpopular).begin(), neighbors(unpopular).end(), popular) != neighbors(unpopular).end();
}

template <typename Func> void Graph::edges_visit(Func&& func) const {
	for (auto const& [n1, neighbors] : views::enumerate(edges)) {
		for (auto const& n2 : neighbors) {
			if (n1 <= n2) {  // Undirected graph
				func(Edge{n1, n2});
			}
		}
	}
}

void Graph::add_edge(Edge edge) {
	edges[edge.first].push_back(edge.second);
	edges[edge.second].push_back(edge.first);
}

void Graph::reserve(std::size_t degree) {
	for (auto& neighborhood : edges) {
		neighborhood.reserve(degree);
	}
}

auto Graph::erdos_renyi(std::size_t n_nodes, double edge_probability, RandomEngine& random_engine) -> Graph {
	// Allocate adjacency lists for the expected approximate number of neighbors in an Erdos Renyi graph.
	// Computed as the expectation of a Binomial.
	auto graph = Graph{n_nodes};
	auto const expected_neighbors = static_cast<std::size_t>(std::ceil(static_cast<double>(n_nodes) * edge_probability));
	graph.reserve(expected_neighbors);

	// Flip a (continuous) coin for each edge in the undirected graph
	auto rand = std::uniform_real_distribution<double>{0.0, 1.0};
	for (Node n1 = 0; n1 < n_nodes; ++n1) {
		for (Node n2 = n1 + 1; n2 < n_nodes; ++n2) {
			if (rand(random_engine) < edge_probability) {
				graph.add_edge({n1, n2});
			}
		}
	}

	return graph;
}

auto Graph::barabasi_albert(std::size_t n_nodes, std::size_t affinity, RandomEngine& random_engine) -> Graph {
	if (affinity < 1 || affinity >= n_nodes) {
		throw std::invalid_argument{"Affinity must be between 1 and the number of nodes."};
	}

	// Allocate adjacency lists for the expected approximate number of neighbors in an Barabasi Albert graph.
	// Computed as the expectation of a power law.
	// https://web.archive.org/web/20200615213344/https://barabasi.com/f/622.pdf
	auto graph = Graph{n_nodes};
	graph.reserve(2 * affinity);

	// First nodes are all connected to the first one (star shape).
	for (Node n = 1; n <= affinity; ++n) {
		graph.add_edge({0, n});
	}

	// Function to get Degrees from 0 to k_nodes (exluded) as vector of doubles.
	auto get_degrees = [&graph](auto k_nodes) {
		auto get_degree = [&graph](auto m) { return static_cast<double>(graph.degree(m)); };
		return views::ints(Node{0}, k_nodes) | views::transform(get_degree) | ranges::to<std::vector>();
	};

	// Other node grow the graph one by one
	for (Node n = affinity + 1; n < n_nodes; ++n) {
		// They are linked to `affinity` existing node with probability proportional to degree
		for (auto neighbor : utility::arg_choice(affinity, get_degrees(affinity), random_engine)) {
			graph.add_edge({n, neighbor});
		}
	}

	return graph;
}

/** Set with comparison function in decreasing order. */
template <typename T> using max_set = std::set<T, std::greater<T>>;

/** Create a set of nodes from 0 to n_nodes */
auto node_set(std::size_t n_nodes) -> max_set<Graph::Node> {
	auto nodes = max_set<Graph::Node>{};
	for (auto n = Graph::Node{0}; n < Graph::Node{n_nodes}; ++n) {
		nodes.insert(n);
	}
	return nodes;
}

/** Find, remove, and return the maximum value from a set. */
template <typename T> auto extract_max(max_set<T>& elements) -> T {
	assert(!elements.empty());
	// std::set are sorted so we get the max with an iterator to the begining (inverse comparasion function).
	return elements.extract(elements.begin()).value();
}

/** Compute intersection between a vector and a set. */
template <typename T>
auto intersection(std::vector<T> const& vect_elems, max_set<T> const& set_elems) -> std::vector<T> {
	auto result = vect_elems;
	auto const in_set = [&set_elems](auto const& x) { return set_elems.find(x) != set_elems.cend(); };
	auto const copied_iter = std::copy_if(vect_elems.cbegin(), vect_elems.cend(), result.begin(), in_set);
	result.resize(static_cast<std::size_t>(std::distance(result.begin(), copied_iter)));
	return result;
}

/** Out of place sort a vector in decreasing order using the given key as comparison. */
template <typename T, typename Key> auto reverse_sorted(std::vector<T> vec, Key&& key) -> std::vector<T> {
	std::sort(vec.begin(), vec.end(), [key](auto const& x, auto const& y) { return key(x) > key(y); });
	return vec;
}

/** Return a vector of Node with the center and an allocated size. */
auto allocate_clique(Graph::Node center, std::size_t n_center_neighors) {
	auto clique = std::vector<Graph::Node>{};
	clique.reserve(n_center_neighors + 1);
	clique.push_back(center);
	return clique;
}

auto Graph::greedy_clique_partition() const -> std::vector<std::vector<Node>> {
	auto clique_partition = std::vector<std::vector<Node>>{};
	clique_partition.reserve(n_nodes());

	auto leftover_nodes = node_set(n_nodes());
	auto const get_degree = [this](Node n) { return degree(n); };

	// Process all nodes to put them in a new clique
	while (!leftover_nodes.empty()) {
		// Start clique from the node with most neighbors
		auto const clique_center = extract_max(leftover_nodes);
		auto const clique_candidates = reverse_sorted(intersection(neighbors(clique_center), leftover_nodes), get_degree);
		auto clique = allocate_clique(clique_center, clique_candidates.size());

		// Candidate clique members are among the neighbors
		for (auto node : clique_candidates) {
			// If clique candidate preserve cliqueness, i.e. connected to every node in clique
			if (std::all_of(
						clique.begin(), clique.end(), [&](auto clique_node) { return are_connected(node, clique_node); })) {
				clique.push_back(node);
				leftover_nodes.extract(node);
			}
		}

		clique_partition.push_back(std::move(clique));
	}

	return clique_partition;
}

/** Make a graph according to the IndependentSetGenerator parameters specifications. */
auto make_graph(IndependentSetGenerator::Parameters parameters, RandomEngine& random_engine) -> Graph {
	switch (parameters.graph_type) {
	case IndependentSetGenerator::Parameters::erdos_renyi:
		return Graph::erdos_renyi(parameters.n_nodes, parameters.edge_probability, random_engine);
	case IndependentSetGenerator::Parameters::barabasi_albert:
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
		for (auto const& [id, clique] : views::enumerate(clique_partition)) {
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

scip::Model IndependentSetGenerator::generate_instance(RandomEngine& random_engine, Parameters parameters) {
	auto const graph = make_graph(parameters, random_engine);
	auto model = scip::Model::prob_basic();
	auto* const scip = model.get_scip_ptr();

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
