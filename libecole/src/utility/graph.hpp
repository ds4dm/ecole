#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include <robin_hood.h>

#include "ecole/export.hpp"
#include "ecole/random.hpp"

namespace ecole::utility {

/** A simple symetric graph based on adjacency lists.  */
class ECOLE_EXPORT Graph {
public:
	using Node = std::size_t;

	struct ECOLE_EXPORT Edge : std::pair<Node, Node> {
		/** All constructors from pair. */
		using std::pair<Node, Node>::pair;

		/** Undirected comparison. */
		ECOLE_EXPORT auto operator==(Edge const& other) const noexcept -> bool;
		ECOLE_EXPORT auto operator!=(Edge const& other) const noexcept -> bool;
	};

	/** Sample a new graph using Erdos Renyi algorithm.
	 *
	 * @param n_nodes The number of nodes in the graph generated.
	 * @param edge_probability The probability that a given edge is added to the graph.
	 * @param random_engine The random number generator used to sample edges.
	 */
	ECOLE_EXPORT static auto erdos_renyi(std::size_t n_nodes, double edge_probability, RandomEngine& random_engine)
		-> Graph;

	/** Sample a new graph using Barabasi Albert algorithm.
	 *
	 * @param n_nodes The number of nodes in the graph generated.
	 * @param affinity The number of nodes that each node is connected to.
	 * @param random_engine The random number generator used to sample edges.
	 */
	ECOLE_EXPORT static auto barabasi_albert(std::size_t n_nodes, std::size_t affinity, RandomEngine& random_engine)
		-> Graph;

	/** Empty graph with only nodes */
	Graph(std::size_t n_nodes) : edges{n_nodes} {}

	/** Reserve size for each adjacency list. */
	ECOLE_EXPORT void reserve(std::size_t degree);

	[[nodiscard]] ECOLE_EXPORT auto n_nodes() const noexcept -> std::size_t;
	[[nodiscard]] ECOLE_EXPORT auto degree(Node n) const noexcept -> std::size_t;
	[[nodiscard]] ECOLE_EXPORT auto neighbors(Node n) const noexcept -> robin_hood::unordered_flat_set<Node> const&;
	[[nodiscard]] ECOLE_EXPORT auto are_connected(Node popular, Node unpopular) const -> bool;
	[[nodiscard]] ECOLE_EXPORT auto n_edges() const noexcept -> std::size_t;

	/** Apply a function on all edges in the graph.
	 *
	 * Iterators care more complex to implement so we provide a visitor pattern.
	 */
	template <typename Func> void edges_visit(Func&& func) const;

	ECOLE_EXPORT void add_edge(Edge edge);

	/** Partition the nodes in clique using greedy algorithm.
	 *
	 * @return Vector of cliques, each being a vector of nodes.
	 */
	[[nodiscard]] ECOLE_EXPORT auto greedy_clique_partition() const -> std::vector<std::vector<Node>>;

private:
	// Vector likely more performant than list on small-sized small-count data due to more predictable cache usage
	using AdjacencyLists = std::vector<robin_hood::unordered_flat_set<Node>>;

	AdjacencyLists edges;
};

/*****************************
 *  Implementation of Graph  *
 *****************************/

template <typename Func> void Graph::edges_visit(Func&& func) const {
	auto const n_nodes_ = n_nodes();
	for (auto n1 = Node{0}; n1 < n_nodes_; ++n1) {
		for (auto n2 : neighbors(n1)) {
			if (n1 <= n2) {  // Undirected graph
				func(Edge{n1, n2});
			}
		}
	}
}

}  // namespace ecole::utility
