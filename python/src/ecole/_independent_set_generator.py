from itertools import combinations
from typing import List, Set, Tuple, Dict

import numpy as np

import ecole.scip


class IndependentSetGenerator:
    def __init__(
        self,
        n_nodes: int = 100,
        edge_probability: float = 0.25,
        affinity: int = 5,
        graph_type: str = "barabasi_albert",
    ):
        """Constructor for the independent set generator.

        The parameters passed in this constructor will be used when a user calls next() or iterates
        over the object.

        Parameters:
        ----------
        n_nodes:
            The number of nodes in the graph.
        edge_probability:
            The probability of generating each edge.
            This parameter must be in the range [0, 1].
            This parameter will only be used if graph_type = "erdos_renyi"
        affinity:
            The number of nodes each new node will be attached to, in the sampling scheme.
            This parameter must be an integer >= 1.
            This parameter will only be used if graph_type = "barabasi_albert".
        graph_type:
            The method used in which to generate graphs.  One of "barabasi_albert" or "erdos_renyi"

        """
        self.n_nodes = n_nodes
        self.edge_probability = edge_probability
        self.affinity = affinity
        self.graph_type = graph_type

        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        """Gets the next instances of a independent set problem.

        This method calls generate_instance() with the parameters passed in
        the constructor and returns the ecole.scip.Model.

        Returns
        -------
        model:
            an ecole model of a independent set instance.

        """

        return generate_instance(
            self.n_nodes, self.edge_probability, self.affinity, self.graph_type, self.rng
        )

    def seed(self, seed: int):
        """Seeds IndependentSetGenerator.

        This method sets the random seed of the IndependentSetGenerator.

        Parameters
        ----------
        seed:
            The seed in which to set the random number generator with.

        """
        self.rng.seed(seed)


def generate_instance(
    n_nodes: int,
    edge_probability: float,
    affinity: int,
    graph_type: str,
    rng: np.random.RandomState,
):
    """

    Generates an instance of an independent set problem.

    This method generates an instance of the independent set problem based on the
    specified parameters and returns it as an ecole model.

    Parameters
    ----------
    n_nodes:
        The number of nodes in the graph.
    edge_probability:
        The probability of generating each edge.
        This parameter must be in the range [0, 1].
        This parameter will only be used if graph_type = "erdos_renyi".
    affinity:
        The number of nodes each new node will be attached to, in the sampling scheme.
        This parameter must be an integer >= 1.
        This parameter will only be used if graph_type = "barabasi_albert".
    graph_type:
        The method used in which to generate graphs.
        This parameter must be one of "barabasi_albert" or "erdos_renyi".
    rng:
        A random number generator.

    Returns
    -------
    model:
        an ecole model of a independent set instance.

    """
    # generate graph
    if graph_type == "barabasi_albert":
        graph = Graph.barabasi_albert(n_nodes, affinity, rng)
    elif graph_type == "erdos_renyi":
        graph = Graph.erdos_renyi(n_nodes, edge_probability, rng)
    else:
        raise Exception("graph_type must be one of 'barabasi_albert' or 'erdos_renyi'")

    cliques = graph.greedy_clique_partition()
    inequalities = set(graph.edges)
    for clique in cliques:
        clique = tuple(sorted(clique))
        for edge in combinations(clique, 2):
            inequalities.remove(edge)
        if len(clique) > 1:
            inequalities.add(clique)

    # Put trivial inequalities for nodes that didn't appear
    # in the constraints, otherwise SCIP will complain
    used_nodes = set()
    for group in inequalities:
        used_nodes.update(group)
    for node in range(10):
        if node not in used_nodes:
            inequalities.add((node,))

    model = ecole.scip.Model.prob_basic()
    pyscipopt_model = model.as_pyscipopt()
    pyscipopt_model.setMaximize()

    var_dict = {}

    # add variable for each node
    for node in range(len(graph)):
        var_dict[node + 1] = pyscipopt_model.addVar(name=f"x_{node+1}", vtype="B", obj=1.0)

    # add constraints such that no connect nodes both set to 1
    for count, group in enumerate(inequalities):
        vars_to_sum = [var_dict[node + 1] for node in sorted(group)]
        cons_lhs = 0
        for var in vars_to_sum:
            cons_lhs += var
        pyscipopt_model.addCons(cons_lhs <= 1)

    return model


class Graph:
    def __init__(
        self,
        n_nodes: int,
        edges: Set[Tuple[int]],
        degrees: np.ndarray,
        neighbors: Dict[int, Set[int]],
    ):
        """Constructor for the Graph container.

        The parameters passed are identify a graph.

        Parameters:
        ----------
        n_nodes:
            The number of nodes in the graph.
        edges:
            A set containing the edges in the graph.
        degrees:
            An array containing the degreee of each node, where
            degrees[node] gives the degree of the node.
        neighbors:
            A dict containing the neighbors of each node, where
            neighbors[node] gives a set of the neighbors.

        """
        self.n_nodes = n_nodes
        self.edges = edges
        self.degrees = degrees
        self.neighbors = neighbors

    def __len__(self):
        """Gets the number of nodes in the graph.

        Returns
        -------
        The number of nodes in the graph.

        """
        return self.n_nodes

    def greedy_clique_partition(self):
        """Partition the graph into cliques using a greedy algorithm.

        Returns
        -------
        cliques:
            The a list of sets containing the cliques found by the algorithm.

        """
        cliques = []
        leftover_nodes = (-self.degrees).argsort().tolist()

        while leftover_nodes:
            clique_center, leftover_nodes = leftover_nodes[0], leftover_nodes[1:]
            clique = {clique_center}
            neighbors = self.neighbors[clique_center].intersection(leftover_nodes)
            densest_neighbors = sorted(neighbors, key=lambda x: -self.degrees[x])
            for neighbor in densest_neighbors:
                # Can you add it to the clique, and maintain cliqueness?
                if all([neighbor in self.neighbors[clique_node] for clique_node in clique]):
                    clique.add(neighbor)
            cliques.append(clique)
            leftover_nodes = [node for node in leftover_nodes if node not in clique]

        return cliques

    @staticmethod
    def erdos_renyi(n_nodes: int, edge_probability: float, rng: np.random.RandomState):
        """Generate an Erdös-Rényi random graph.

        This method is used to generate an Erdös-Rényi graph by randomly adding edges with
        the specified probability.

        Parameters
        ----------
        n_nodes:
            The number of nodes in the graph.
        edge_probability:
            The probability of generating each edge.
            This value must be bound in the range [0,1].
        rng:
            A random number generator.

        Returns
        -------
        Graph:
            The generated graph.

        """
        edges = set()
        degrees = np.zeros(n_nodes, dtype=int)
        neighbors = {node: set() for node in range(n_nodes)}
        for edge in combinations(np.arange(n_nodes), 2):
            if rng.uniform() < edge_probability:
                edges.add(edge)
                degrees[edge[0]] += 1
                degrees[edge[1]] += 1
                neighbors[edge[0]].add(edge[1])
                neighbors[edge[1]].add(edge[0])
        graph = Graph(n_nodes, edges, degrees, neighbors)
        return graph

    @staticmethod
    def barabasi_albert(n_nodes: int, affinity: int, rng: np.random.RandomState):
        """Generate a Barabási-Albert random graph.

        This method is used to generate a Barabási-Albert graph based on the specified affinity.

        Parameters
        ----------
        n_nodes:
            The number of nodes in the graph.
        affinity:
            The number of nodes each new node will be attached to, in the sampling scheme.
            This parameter must be an integer >= 1.
        rng:
            A random number generator.

        Returns
        -------
        Graph:
            The generated graph.

        """
        assert affinity >= 1 and affinity < n_nodes

        edges = set()
        degrees = np.zeros(n_nodes, dtype=int)
        neighbors = {node: set() for node in range(n_nodes)}
        for new_node in range(affinity, n_nodes):
            # first node is connected to all previous ones (star-shape)
            if new_node == affinity:
                neighborhood = np.arange(new_node)
            # remaining nodes are picked stochastically
            else:
                neighbor_prob = degrees[:new_node] / (2 * len(edges))
                neighborhood = rng.choice(new_node, affinity, replace=False, p=neighbor_prob)
            for node in neighborhood:
                edges.add((node, new_node))
                degrees[node] += 1
                degrees[new_node] += 1
                neighbors[node].add(new_node)
                neighbors[new_node].add(node)

        graph = Graph(n_nodes, edges, degrees, neighbors)
        return graph
