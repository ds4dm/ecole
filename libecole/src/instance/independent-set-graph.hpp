#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "ecole/random.hpp"

namespace ecole::instance {

/** A simple symetric graph based on adjacency lists.  */
class Graph {
public:
	using Node = std::size_t;

	struct Edge : std::pair<Node, Node> {
		/** All constructors from pair. */
		using std::pair<Node, Node>::pair;

		/** Undirected comparison. */
		auto operator==(Edge const& other) const noexcept -> bool;
		auto operator!=(Edge const& other) const noexcept -> bool { return !(*this == other); }
	};

	/** Sample a new graph using Erdos Renyi algorithm.
	 *
	 * @param n_nodes The number of nodes in the graph generated.
	 * @param edge_probability The probability that a given edge is added to the graph.
	 * @param random_engine The random number generator used to sample edges.
	 */
	static auto erdos_renyi(std::size_t n_nodes, double edge_probability, RandomEngine& random_engine) -> Graph;

	/** Sample a new graph using Barabasi Albert algorithm.
	 *
	 * @param n_nodes The number of nodes in the graph generated.
	 * @param affinity The number of nodes that each node is connected to.
	 * @param random_engine The random number generator used to sample edges.
	 */
	static auto barabasi_albert(std::size_t n_nodes, std::size_t affinity, RandomEngine& random_engine) -> Graph;

	/** Empty graph with only nodes */
	Graph(std::size_t n_nodes) : edges{n_nodes} {}

	/** Reserve size for each adjacency list. */
	void reserve(std::size_t degree);

	[[nodiscard]] auto n_nodes() const noexcept -> std::size_t { return edges.size(); }
	[[nodiscard]] auto degree(Node n) const noexcept -> std::size_t { return edges[n].size(); }
	[[nodiscard]] auto neighbors(Node n) const noexcept -> std::vector<Node> const& { return edges[n]; }
	[[nodiscard]] auto are_connected(Node popular, Node unpopular) const -> bool;
	[[nodiscard]] auto n_edges() const noexcept -> std::size_t;

	/** Apply a function on all edges in the graph.
	 *
	 * Iterators care more complex to implement so we provide a visitor pattern.
	 */
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

}  // namespace ecole::instance
