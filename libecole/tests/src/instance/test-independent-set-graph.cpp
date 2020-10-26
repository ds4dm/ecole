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
