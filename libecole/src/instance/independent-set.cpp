#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include "ecole/instance/independent-set.hpp"
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

/**
 * A simple symetric graph based on adjacency lists.
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

	void add_edge(Edge edge);

	// [[nodiscard]] auto greedy_clique_pratition() const -> Foo;

private:
	using AdjacencyLists = std::vector<std::vector<Node>>;

	AdjacencyLists edges;

	Graph(AdjacencyLists edges_) : edges(std::move(edges_)) {}
};

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
	// Allocate adjacency lists for the expected approximate number of neighbours in an Erdos Renyi graph.
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

	// Allocate adjacency lists for the expected approximate number of neighbours in an Barabasi Albert graph.
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

}  // namespace

scip::Model IndependentSetGenerator::generate_instance(RandomEngine& /*random_engine*/, Parameters /*parameters*/) {
	// TODO implement algorithm
	return {};
}

}  // namespace ecole::instance
