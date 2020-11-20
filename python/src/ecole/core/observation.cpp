#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/observation/pseudocosts.hpp"
#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "core.hpp"

namespace ecole::observation {

namespace py = pybind11;

/**
 * Helper function to bind the `before_reset` method of observation functions.
 */
template <typename PyClass, typename... Args> auto def_before_reset(PyClass pyclass, Args&&... args) {
	return pyclass.def(
		"before_reset",
		&PyClass::type::before_reset,
		py::arg("model"),
		py::call_guard<py::gil_scoped_release>(),
		std::forward<Args>(args)...);
}

/**
 * Helper function to bind the `extract` method of observation functions.
 */
template <typename PyClass, typename... Args> auto def_extract(PyClass pyclass, Args&&... args) {
	return pyclass.def(
		"extract",
		&PyClass::type::extract,
		py::arg("model"),
		py::arg("done"),
		py::call_guard<py::gil_scoped_release>(),
		std::forward<Args>(args)...);
}

/**
 * Observation module bindings definitions.
 */
void bind_submodule(py::module_ const& m) {
	m.doc() = "Observation classes for Ecole.";

	xt::import_numpy();

	m.attr("Nothing") = py::type::of<Nothing>();

	using coo_matrix = decltype(NodeBipartiteObs::edge_features);
	py::class_<coo_matrix>(m, "coo_matrix", R"(
		Sparse matrix in the coordinate format.

		Similar to Scipy's ``scipy.sparse.coo_matrix`` or PyTorch ``torch.sparse``.
	)")
		.def_property_readonly(
			"values", [](coo_matrix & self) -> auto& { return self.values; }, "A vector of non zero values in the matrix")
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
	def_before_reset(node_bipartite, "Cache some feature not expected to change during an episode.");
	def_extract(node_bipartite, "Extract a new :py:class:`NodeBipartiteObs`.");

	auto strong_branching_scores = py::class_<StrongBranchingScores>(m, "StrongBranchingScores", R"(
		Strong branching score observation function on branch-and bound node.

		This observation obtains scores for all LP or pseudo candidate variables at a
		branch-and-bound node.  The strong branching score measures the quality of branching
		for each variable.  This observation can be used as an expert for imitation
		learning algorithms.

		This observation function extracts an array containing the strong branching score for
		each variable in the problem which can be indexed by the action set.  Variables for which
		a strong branching score is not applicable are filled with NaN.
	)");
	strong_branching_scores.def(py::init<bool>(), py::arg("pseudo_candidates") = true, R"(
		Constructor for StrongBranchingScores.

		Parameters
		----------
		pseudo_candidates :
			The parameter determines if strong branching scores are computed for
			psuedo-candidate variables if true or LP canidate variables if false.
			By default psuedo-candidates will be computed.
	)");
	def_before_reset(strong_branching_scores, R"(Do nothing.)");
	def_extract(strong_branching_scores, "Extract an array containing strong branching scores.");

	auto pseudocosts = py::class_<Pseudocosts>(m, "Pseudocosts", R"(
		Pseudocosts observation function on branch-and bound node.

		This observation obtains pseudocosts for all LP fractional candidate variables at a
		branch-and-bound node.  The pseudocost is a cheap approximation to the strong branching
		score and measures the quality of branching for each variable.  This observation can be used
		as a practical branching strategy by always branching on the variable with the highest
		pseudocost, although in practice is it not as efficient as SCIP's default strategy, reliability
		pseudocost branching (also known as hybrid branching).

		This observation function extracts an array containing the pseudocost for
		each variable in the problem which can be indexed by the action set.  Variables for which
		a pseudocost is not applicable are filled with NaN.
	)");
	pseudocosts.def(py::init<>());
	def_before_reset(pseudocosts, R"(Do nothing.)");
	def_extract(pseudocosts, "Extract an array containing pseudocosts.");
}

}  // namespace ecole::observation
