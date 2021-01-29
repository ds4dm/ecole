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

#include "instance/independent-set-graph.hpp"

namespace views = ranges::views;

namespace ecole::instance {

auto Graph::Edge::operator==(Edge const& other) const noexcept -> bool {
	return ((first == other.first) && (second == other.second)) || ((first == other.second) && (second == other.first));
}

auto Graph::are_connected(Node popular, Node unpopular) const -> bool {
	// We search in neighborhood of unpopular node rather than popular to reduce compexity of linear scan
	return std::find(neighbors(unpopular).begin(), neighbors(unpopular).end(), popular) != neighbors(unpopular).end();
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
		for (auto neighbor : utility::arg_choice(affinity, get_degrees(n), random_engine)) {
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
	auto const get_degree = [this](Graph::Node n) { return degree(n); };

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

}  // namespace ecole::instance
