#include <memory>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse_matrix.hpp"

#include "core.hpp"

namespace ecole {
namespace observation {

namespace py = pybind11;

/**
 * Helper function to bind the `reset` method of observation functions.
 */
template <typename PyClass, typename... Args> auto def_reset(PyClass pyclass, Args&&... args) {
	return pyclass.def(
		"reset",
		&PyClass::type::reset,
		py::arg("model"),
		py::call_guard<py::gil_scoped_release>(),
		std::forward<Args>(args)...);
}

/**
 * Helper function to bind the `obtain_observation` method of observation functions.
 */
template <typename PyClass, typename... Args>
auto def_obtain_observation(PyClass pyclass, Args&&... args) {
	return pyclass.def(
		"obtain_observation",
		&PyClass::type::obtain_observation,
		py::arg("model"),
		py::call_guard<py::gil_scoped_release>(),
		std::forward<Args>(args)...);
}

/**
 * Observation module bindings definitions.
 */
void bind_submodule(py::module m) {
	m.doc() = "Observation classes for Ecole.";

	xt::import_numpy();

	auto nothing = py::class_<Nothing>(m, "Nothing", R"(
		No observation.

		This observation function does nothing and always returns ``None`` as an observation.
		Convenient for bandit algorithms, or when no learning is performed.
	)");
	nothing.def(py::init<>());
	def_reset(nothing, R"(Do nothing.)");
	def_obtain_observation(nothing, R"(Return None.)");

	using coo_matrix = decltype(NodeBipartiteObs::edge_features);
	py::class_<coo_matrix>(m, "coo_matrix", R"(
		Sparse matrix in the coordinate format.

		Similar to Scipy's ``scipy.sparse.coo_matrix`` or PyTorch ``torch.sparse``.
	)")
		.def_property_readonly(
			"values",
			[](coo_matrix & self) -> auto& { return self.values; },
			"A vector of non zero values in the matrix")
		.def_property_readonly(
			"indices",
			[](coo_matrix & self) -> auto& { return self.indices; },
			"A matrix holding the indices of non zero coefficient in the sparse matrix. "
			"There are as many columns as there are non zero coefficients, and each row is a "
			"dimension in the sparse matrix.")
		.def_property_readonly(
			"shape",
			[](coo_matrix& self) { return std::make_pair(self.shape[0], self.shape[1]); },
			"The dimension of the sparse matrix, as if it was dense.")
		.def_property_readonly("nnz", &coo_matrix::nnz);

	py::class_<NodeBipartiteObs>(m, "NodeBipartiteObs", R"(
		Bipartite graph observation for branch-and-bound nodes.

		The optimization problem is represented as an heterogenous bipartite graph.
		On one side, a node is associated with one variable, on the other side a node is
		associated with one constraint.
		There exist an edge between a variable and a constraint if the variable exists in the
		constraint with a non-zero coefficient.

		Each variable and constraint node is associated with a vector of features.
		Each edge is associated with the coefficient of the variable in the constraint.
	)")  //
		.def_property_readonly(
			"column_features",
			[](NodeBipartiteObs & self) -> auto& { return self.column_features; },
			"A matrix where each row is represents a variable, and each column a feature of "
			"the variables.")
		.def_property_readonly(
			"row_features",
			[](NodeBipartiteObs & self) -> auto& { return self.row_features; },
			"A matrix where each row is represents a constraint, and each column a feature of "
			"the constraints.")
		.def_readwrite(
			"edge_features",
			&NodeBipartiteObs::edge_features,
			"The constraint matrix of the optimization problem, with rows for contraints and "
			"columns for variables.");

	auto node_bipartite = py::class_<NodeBipartite>(m, "NodeBipartite", R"(
		Bipartite graph observation function on branch-and bound node.

		This observation function extract structured :py:class:`NodeBipartiteObs`.
	)");
	node_bipartite.def(py::init<>());
	def_reset(node_bipartite, "Cache some feature not expected to change during an episode.");
	def_obtain_observation(node_bipartite, "Extract a new :py:class:`NodeBipartiteObs`.");

	py::class_<StrongBranchingScoresObs>(m, "StrongBranchingScoresObs", R"(
		Strong Branching Scores for each variable at a branch-and-bound node.

		This observation obtains scores for all LP or pseudo candidate variables at a 
		branch-and-bound node.
		The strong branching score measures the quality of branching for each variable.
		This observation can be used as an expert in reinforcement learning algorithms. 
	)")  //
		.def_property_readonly(
			"strong_branching_scores", [](StrongBranchingScoresObs & self) -> auto& { return self.strong_branching_scores; });

		This observation obtains scores for all LP or pseudo candidate variables at a 
		branch-and-bound node.  The strong branching score measures the quality of branching 
		for each variable.  This observation can be used as an expert in reinforcement
		learning algorithms. 
	)")  //
		.def_property_readonly(
			"scores",
			[](StrongBranchingScoresObs & self) -> auto& { return self.scores; },
			"An array containing the strong branching score for each variable in the problem "
			"which can be indexed by the action set.  Variables for which a strong branching "
			"score is not applicable are filled with NaN.");

	auto strong_branching_scores =
		py::class_<StrongBranchingScores>(m, "StrongBranchingScores", R"(
		Strong branching score observation function on branch-and bound node.  The parameter
		pseudo_candidates can be passed as a boolean to compute strong branching scores for 
		pseudo-candidate variables if true and LP candidates variables if false.  By default
		psuedo-candidates will be computed.  

		This observation function extract structured :py:class:`StrongBranchingScoresObs`.
	)");
	strong_branching_scores.def(py::init<bool>(), py::arg("pseudo_candidates") = true);
	def_reset(
		strong_branching_scores,
		"Cache some feature not expected to change during an episode.");
	def_obtain_observation(
		strong_branching_scores, "Extract a new :py:class:`StrongBranchingScoresObs`.");
}

}  // namespace observation
}  // namespace ecole
