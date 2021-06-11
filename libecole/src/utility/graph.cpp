#include <algorithm>
#include <functional>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include "ecole/utility/random.hpp"

#include "utility/graph.hpp"

namespace views = ranges::views;

namespace ecole::utility {

auto Graph::Edge::operator==(Edge const& other) const noexcept -> bool {
	return ((first == other.first) && (second == other.second)) || ((first == other.second) && (second == other.first));
}

auto Graph::are_connected(Node popular, Node unpopular) const -> bool {
	return neighbors(unpopular).contains(popular);
}

auto Graph::n_edges() const noexcept -> std::size_t {
	auto count = std::size_t{0};
	for (auto const& neighbors : edges) {
		count += neighbors.size();
	}
	// Each edge is stored twice
	assert(count % 2 == 0);
	return count / 2;
}

void Graph::add_edge(Edge edge) {
	assert(!are_connected(edge.first, edge.second));
	edges[edge.first].insert(edge.second);
	edges[edge.second].insert(edge.first);
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
		for (auto neighbor : utility::arg_choice(affinity, get_degrees(n), random_engine)) {
			graph.add_edge({n, neighbor});
		}
	}

	return graph;
}

template <typename K, typename V> using map = robin_hood::unordered_flat_map<K, V>;
template <typename T> using set = robin_hood::unordered_flat_set<T>;

/** Create a set of mapping of nodes to their degrees. */
auto create_nodes_degrees(Graph const& g) -> map<Graph::Node, std::size_t> {
	auto nodes = map<Graph::Node, std::size_t>{};
	nodes.reserve(g.n_nodes());
	for (auto n = Graph::Node{0}; n < Graph::Node{g.n_nodes()}; ++n) {
		nodes[n] = g.degree(n);
	}
	return nodes;
}

/** Find, remove, and return the node with maximum degree. */
auto extract_node_with_max_degree(map<Graph::Node, std::size_t>& nodes_degrees) -> Graph::Node {
	assert(!nodes_degrees.empty());
	auto cmp_degrees = [](auto const& nd_1, auto const& nd_2) { return nd_1.second < nd_2.second; };
	auto const max_iter = std::max_element(nodes_degrees.begin(), nodes_degrees.end(), cmp_degrees);
	auto node = max_iter->first;
	nodes_degrees.erase(max_iter);
	return node;
}

/** Compute intersection between neighborhood and leftovers nodes and sort them by decreasing degree. */
auto best_clique_candidates(set<Graph::Node> const& neighborhood, map<Graph::Node, std::size_t> const& leftover_nodes)
	-> std::vector<Graph::Node> {
	auto candidates = std::vector<Graph::Node>{};
	candidates.reserve(std::min(neighborhood.size(), leftover_nodes.size()));
	auto in_leftover_nodes = [&leftover_nodes](auto node) { return leftover_nodes.contains(node); };
	std::copy_if(neighborhood.begin(), neighborhood.end(), std::back_inserter(candidates), in_leftover_nodes);
	// Decreasing sort by degree using > comparison.
	auto cmp_degrees = [&leftover_nodes](auto node1, auto node2) {
		assert(leftover_nodes.contains(node1) && leftover_nodes.contains(node2));
		return leftover_nodes.find(node1)->second > leftover_nodes.find(node2)->second;
	};
	std::sort(candidates.begin(), candidates.end(), cmp_degrees);
	return candidates;
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

	auto leftover_nodes = create_nodes_degrees(*this);

	// Process all nodes to put them in a new clique
	while (!leftover_nodes.empty()) {
		// Start clique from the node with most neighbors
		auto const clique_center = extract_node_with_max_degree(leftover_nodes);
		auto const clique_candidates = best_clique_candidates(neighbors(clique_center), leftover_nodes);
		auto clique = allocate_clique(clique_center, clique_candidates.size());

		// Candidate clique members are among the neighbors
		for (auto node : clique_candidates) {
			// If clique candidate preserve cliqueness, i.e. connected to every node in clique
			if (std::all_of(
						clique.begin(), clique.end(), [&](auto clique_node) { return are_connected(node, clique_node); })) {
				clique.push_back(node);
				[[maybe_unused]] auto const n_removed = leftover_nodes.erase(node);
				assert(n_removed == 1);
			}
		}

		clique_partition.push_back(std::move(clique));
	}

	return clique_partition;
}

}  // namespace ecole::utility
