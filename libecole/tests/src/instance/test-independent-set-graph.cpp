#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <catch2/catch.hpp>

#include "instance/independent-set-graph.hpp"
#include "instance/unit-tests.hpp"

using namespace ecole;
using Graph = instance::Graph;
using Edge = Graph::Edge;

template <typename Container>
auto contains(Container const& container, typename Container::value_type const& val) -> bool {
	return std::find(container.begin(), container.end(), val) != container.end();
}

TEST_CASE("Edge comparison", "[instance][unit]") {
	REQUIRE(Edge{0, 1} == Edge{1, 0});
	REQUIRE(Edge{0, 1} != Edge{0, 0});
}

TEST_CASE("Unit test graph class used in IndependentSet", "[instance][unit]") {
	std::size_t constexpr n_nodes = 4;
	auto constexpr edges = std::array{Edge{0, 1}, Edge{2, 0}};
	auto graph = Graph{n_nodes};
	std::for_each(edges.begin(), edges.end(), [&graph](auto edge) { graph.add_edge(edge); });

	SECTION("Graph builders") {
		auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests

		SECTION("Erdos Renyi") {
			auto constexpr edge_prob = 0.9;
			graph = Graph::erdos_renyi(n_nodes, edge_prob, random_engine);
		}

		SECTION("Barabasi Albert") {
			auto constexpr affinity = 1;
			graph = Graph::barabasi_albert(n_nodes, affinity, random_engine);
		}

		REQUIRE(graph.n_nodes() == n_nodes);
	}

	SECTION("Get the number of nodes") { REQUIRE(graph.n_nodes() == n_nodes); }

	SECTION("Get the number of edges") { REQUIRE(graph.n_edges() == edges.size()); }

	SECTION("Get degrees") {
		REQUIRE(graph.degree(0) == 2);
		REQUIRE(graph.degree(1) == 1);
		REQUIRE(graph.degree(2) == 1);
		REQUIRE(graph.degree(3) == 0);
	}

	SECTION("Get neighbors") {
		for (auto [n1, n2] : edges) {
			REQUIRE(contains(graph.neighbors(n1), n2));
			REQUIRE(contains(graph.neighbors(n2), n1));
		}
	}

	SECTION("Check if nodes are connected") {
		for (auto [n1, n2] : edges) {
			REQUIRE(graph.are_connected(n1, n2));
		}
	}

	SECTION("Edge visitor visit edges excatly once") {
		auto edge_count = std::size_t{0};
		graph.edges_visit([&edge_count](auto /* edge */) { edge_count++; });
		REQUIRE(edge_count == edges.size());
	}

	SECTION("Edge visitor visit correct edges") {
		graph.edges_visit([&edges](auto edge) { REQUIRE(contains(edges, edge)); });
	}

	SECTION("Greedy clique partition is a partition") {
		auto node_seen = std::vector<bool>(n_nodes, false);
		auto const cliques = graph.greedy_clique_partition();
		for (auto const& clique : cliques) {
			for (auto const node : clique) {
				node_seen[node] = true;
			}
		}
		REQUIRE(std::all_of(node_seen.begin(), node_seen.end(), [](auto x) { return x; }));
	}

	SECTION("Greedy clique partitions are cliques") {
		auto const cliques = graph.greedy_clique_partition();
		for (auto const& clique : cliques) {
			auto iter1 = clique.begin();
			while (iter1 != clique.end()) {
				auto iter2 = iter1 + 1;
				while (iter2 != clique.end()) {
					REQUIRE(graph.are_connected(*iter1, *(iter2++)));
				}
				++iter1;
			}
		}
	}
}

TEST_CASE("Erdos Renyi builder", "[instance]") {
	// These tests are actually not random because the random engine is always the same, but it could be changed that
	// the results should hold with very high probability
	auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
	auto constexpr n_nodes = 100;
	auto constexpr edge_prob = 0.5;
	auto graph = Graph::erdos_renyi(n_nodes, edge_prob, random_engine);

	// Number of edges follows a binomial(C(n_nodes,2), edge_prob).
	// With the Chernov Bounds, we compute that this is true with proba ~ 1-1e-40
	auto constexpr likely_edge_bound = 2000;
	REQUIRE(graph.n_edges() >= likely_edge_bound);
	REQUIRE(graph.n_edges() <= n_nodes * (n_nodes - 1) - likely_edge_bound);

	// Node degree follows a binomial(n_nodes-1, edge_prob).
	// With the Chernov Bounds, we compute that this is true with proba ~ 1-1e-16
	auto constexpr likely_degree_bound = 10;
	for (auto node = Graph::Node{0}; node < graph.n_nodes(); ++node) {
		REQUIRE(graph.degree(node) >= likely_degree_bound);
		REQUIRE(graph.degree(node) <= n_nodes - 1 - likely_degree_bound);
	}
}

TEST_CASE("Barabasi Albert builder", "[instance]") {
	auto random_engine = RandomEngine{};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducible in tests
	auto constexpr n_nodes = 100;
	auto constexpr affinity = 11;
	auto graph = Graph::barabasi_albert(n_nodes, affinity, random_engine);
	// Deterministic, according to building algorithm
	REQUIRE(graph.n_edges() == (n_nodes - affinity - 1) * affinity + affinity);
}
