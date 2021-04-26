#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/khalil-2016.hpp"
#include "ecole/observation/milpbipartite.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/observation/pseudocosts.hpp"
#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "auto-class.hpp"
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
	auto_class<coo_matrix>(m, "coo_matrix", R"(
		Sparse matrix in the coordinate format.

		Similar to Scipy's ``scipy.sparse.coo_matrix`` or PyTorch ``torch.sparse``.
	)")
		.def_auto_copy()
		.def_auto_pickle(std::array{"values", "indices", "shape"})
		.def_readwrite_xtensor("values", &coo_matrix::values, "A vector of non zero values in the matrix")
		.def_readwrite_xtensor("indices", &coo_matrix::indices, R"(
			A matrix holding the indices of non zero coefficient in the sparse matrix.

			There are as many columns as there are non zero coefficients, and each row is a
			dimension in the sparse matrix.
		)")
		.def_readwrite("shape", &coo_matrix::shape, "The dimension of the sparse matrix, as if it was dense.")
		.def_property_readonly("nnz", &coo_matrix::nnz);

	// Node bipartite observation
	auto node_bipartite_obs =
		auto_class<NodeBipartiteObs>(m, "NodeBipartiteObs", R"(
		Bipartite graph observation for branch-and-bound nodes.

		The optimization problem is represented as an heterogenous bipartite graph.
		On one side, a node is associated with one variable, on the other side a node is
		associated with one constraint.
		There exist an edge between a variable and a constraint if the variable exists in the
		constraint with a non-zero coefficient.

		Each variable and constraint node is associated with a vector of features.
		Each edge is associated with the coefficient of the variable in the constraint.
	)")
			.def_auto_copy()
			.def_auto_pickle(std::array{"column_features", "row_features", "edge_features"})
			.def_readwrite_xtensor(
				"column_features",
				&NodeBipartiteObs::column_features,
				"A matrix where each row is represents a variable, and each column a feature of the variables.")
			.def_readwrite_xtensor(
				"row_features",
				&NodeBipartiteObs::row_features,
				"A matrix where each row is represents a constraint, and each column a feature of the constraints.")
			.def_readwrite(
				"edge_features",
				&NodeBipartiteObs::edge_features,
				"The constraint matrix of the optimization problem, with rows for contraints and "
				"columns for variables.");

	py::enum_<NodeBipartiteObs::ColumnFeatures>(node_bipartite_obs, "ColumnFeatures")
		.value("objective", NodeBipartiteObs::ColumnFeatures::objective)
		.value("is_type_binary", NodeBipartiteObs::ColumnFeatures::is_type_binary)
		.value("is_type_integer", NodeBipartiteObs::ColumnFeatures::is_type_integer)
		.value("is_type_implicit_integer", NodeBipartiteObs::ColumnFeatures::is_type_implicit_integer)
		.value("is_type_continuous", NodeBipartiteObs::ColumnFeatures::is_type_continuous)
		.value("has_lower_bound", NodeBipartiteObs::ColumnFeatures::has_lower_bound)
		.value("has_upper_bound", NodeBipartiteObs::ColumnFeatures::has_upper_bound)
		.value("normed_reduced_cost", NodeBipartiteObs::ColumnFeatures::normed_reduced_cost)
		.value("solution_value", NodeBipartiteObs::ColumnFeatures::solution_value)
		.value("solution_frac", NodeBipartiteObs::ColumnFeatures::solution_frac)
		.value("is_solution_at_lower_bound", NodeBipartiteObs::ColumnFeatures::is_solution_at_lower_bound)
		.value("is_solution_at_upper_bound", NodeBipartiteObs::ColumnFeatures::is_solution_at_upper_bound)
		.value("scaled_age", NodeBipartiteObs::ColumnFeatures::scaled_age)
		.value("incumbent_value", NodeBipartiteObs::ColumnFeatures::incumbent_value)
		.value("average_incumbent_value", NodeBipartiteObs::ColumnFeatures::average_incumbent_value)
		.value("is_basis_lower", NodeBipartiteObs::ColumnFeatures::is_basis_lower)
		.value("is_basis_basic", NodeBipartiteObs::ColumnFeatures::is_basis_basic)
		.value("is_basis_upper", NodeBipartiteObs::ColumnFeatures::is_basis_upper)
		.value("is_basis_zero", NodeBipartiteObs::ColumnFeatures ::is_basis_zero);

	py::enum_<NodeBipartiteObs::RowFeatures>(node_bipartite_obs, "RowFeatures")
		.value("bias", NodeBipartiteObs::RowFeatures::bias)
		.value("objective_cosine_similarity", NodeBipartiteObs::RowFeatures::objective_cosine_similarity)
		.value("is_tight", NodeBipartiteObs::RowFeatures::is_tight)
		.value("dual_solution_value", NodeBipartiteObs::RowFeatures::dual_solution_value)
		.value("scaled_age", NodeBipartiteObs::RowFeatures::scaled_age);

	auto node_bipartite = py::class_<NodeBipartite>(m, "NodeBipartite", R"(
		Bipartite graph observation function on branch-and bound node.

		This observation function extract structured :py:class:`NodeBipartiteObs`.
	)");
	node_bipartite.def(py::init<bool>(), py::arg("cache") = false, R"(
		Constructor for NodeBipartite.

		Parameters
		----------
		cache :
			Whether or not to cache static features within an episode.
			Currently, this is only safe if cutting planes are disabled.
	)");
	def_before_reset(node_bipartite, "Cache some feature not expected to change during an episode.");
	def_extract(node_bipartite, "Extract a new :py:class:`NodeBipartiteObs`.");

	// MILP bipartite observation
	auto milp_bipartite_obs =
		auto_class<MilpBipartiteObs>(m, "MilpBipartiteObs", R"(
		Bipartite graph observation that represents the most recent MILP during presolving.

		The optimization problem is represented as an heterogenous bipartite graph.
		On one side, a node is associated with one variable, on the other side a node is
		associated with one constraint.
		There exist an edge between a variable and a constraint if the variable exists in the
		constraint with a non-zero coefficient.

		Each variable and constraint node is associated with a vector of features.
		Each edge is associated with the coefficient of the variable in the constraint.
	)")
			.def_auto_copy()
			.def_auto_pickle(std::array{"variable_features", "constraint_features", "edge_features"})
			.def_readwrite_xtensor(
				"variable_features",
				&MilpBipartiteObs::variable_features,
				"A matrix where each row represents a variable, and each column a feature of the variables.")
			.def_readwrite_xtensor(
				"constraint_features",
				&MilpBipartiteObs::constraint_features,
				"A matrix where each row is represents a constraint, and each column a feature of the constraints.")
			.def_readwrite(
				"edge_features",
				&MilpBipartiteObs::edge_features,
				"The constraint matrix of the optimization problem, with rows for contraints and columns for variables.");

	py::enum_<MilpBipartiteObs::VariableFeatures>(milp_bipartite_obs, "VariableFeatures")
		.value("objective", MilpBipartiteObs::VariableFeatures::objective)
		.value("is_type_binary", MilpBipartiteObs::VariableFeatures::is_type_binary)
		.value("is_type_integer", MilpBipartiteObs::VariableFeatures::is_type_integer)
		.value("is_type_implicit_integer", MilpBipartiteObs::VariableFeatures::is_type_implicit_integer)
		.value("is_type_continuous", MilpBipartiteObs::VariableFeatures::is_type_continuous)
		.value("has_lower_bound", MilpBipartiteObs::VariableFeatures::has_lower_bound)
		.value("has_upper_bound", MilpBipartiteObs::VariableFeatures::has_upper_bound)
		.value("lower_bound", MilpBipartiteObs::VariableFeatures::lower_bound)
		.value("upper_bound", MilpBipartiteObs::VariableFeatures::upper_bound);

	py::enum_<MilpBipartiteObs::ConstraintFeatures>(milp_bipartite_obs, "ConstraintFeatures")
		.value("bias", MilpBipartiteObs::ConstraintFeatures::bias);

	auto milp_bipartite = py::class_<MilpBipartite>(m, "MilpBipartite", R"(
		Bipartite graph observation function for the sub-MILP at the latest branch-and-bound node.

		This observation function extract structured :py:class:`MilpBipartiteObs`.
	)");
	milp_bipartite.def(py::init<bool>(), py::arg("normalize") = false, R"(
		Constructor for MilpBipartite.

		Parameters
		----------
		use_normalization :
			Should the features be normalized?
			This is recommended for some application such as deep learning models.
	)");
	def_before_reset(milp_bipartite, R"(Do nothing.)");
	def_extract(milp_bipartite, "Extract a new :py:class:`MilpBipartiteObs`.");

	// Strong branching observation
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

	// Pseudocosts observation
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

	// Khalil observation
	auto khalil2016_obs =
		auto_class<Khalil2016Obs>(m, "Khalil2016Obs", R"(
		Branching candidates features from Khalil et al. (2016).

		The observation is a matrix where rows represent pseudo branching candidates and columns
		represent features related to these variables.
		See [Khalil2016]_ for a complete reference on this observation function.

		The first :py:attr:`Khalil2016Obs.n_static_features` are static (they do not change through the solving
		process), and the remaining :py:attr:`Khalil2016Obs.n_dynamic_features` dynamic.

		.. [Khalil2016]
			Khalil, Elias Boutros, Pierre Le Bodic, Le Song, George Nemhauser, and Bistra Dilkina.
			"`Learning to branch in mixed integer programming.
			<https://web.archive.org/web/20200812151256/https://www.cc.gatech.edu/~lsong/papers/KhaLebSonNemDil16.pdf>`_"
			*Thirtieth AAAI Conference on Artificial Intelligence*. 2016.
	)")
			.def_auto_copy()
			.def_auto_pickle(std::array{"features"})
			.def_readwrite_xtensor(
				"features",
				&Khalil2016Obs::features,
				"A matrix where each row represents a variable, and each column a feature of the variables.")
			.def_readonly_static("n_static_features", &Khalil2016Obs::n_static_features)
			.def_readonly_static("n_dynamic_features", &Khalil2016Obs::n_dynamic_features);

	py::enum_<Khalil2016Obs::Features>(khalil2016_obs, "Features")
		.value("obj_coef", Khalil2016Obs::Features::obj_coef)
		.value("obj_coef_pos_part", Khalil2016Obs::Features::obj_coef_pos_part)
		.value("obj_coef_neg_part", Khalil2016Obs::Features::obj_coef_neg_part)
		.value("n_rows", Khalil2016Obs::Features::n_rows)
		.value("rows_deg_mean", Khalil2016Obs::Features::rows_deg_mean)
		.value("rows_deg_stddev", Khalil2016Obs::Features::rows_deg_stddev)
		.value("rows_deg_min", Khalil2016Obs::Features::rows_deg_min)
		.value("rows_deg_max", Khalil2016Obs::Features::rows_deg_max)
		.value("rows_pos_coefs_count", Khalil2016Obs::Features::rows_pos_coefs_count)
		.value("rows_pos_coefs_mean", Khalil2016Obs::Features::rows_pos_coefs_mean)
		.value("rows_pos_coefs_stddev", Khalil2016Obs::Features::rows_pos_coefs_stddev)
		.value("rows_pos_coefs_min", Khalil2016Obs::Features::rows_pos_coefs_min)
		.value("rows_pos_coefs_max", Khalil2016Obs::Features::rows_pos_coefs_max)
		.value("rows_neg_coefs_count", Khalil2016Obs::Features::rows_neg_coefs_count)
		.value("rows_neg_coefs_mean", Khalil2016Obs::Features::rows_neg_coefs_mean)
		.value("rows_neg_coefs_stddev", Khalil2016Obs::Features::rows_neg_coefs_stddev)
		.value("rows_neg_coefs_min", Khalil2016Obs::Features::rows_neg_coefs_min)
		.value("rows_neg_coefs_max", Khalil2016Obs::Features::rows_neg_coefs_max)
		.value("slack", Khalil2016Obs::Features::slack)
		.value("ceil_dist", Khalil2016Obs::Features::ceil_dist)
		.value("pseudocost_up", Khalil2016Obs::Features::pseudocost_up)
		.value("pseudocost_down", Khalil2016Obs::Features::pseudocost_down)
		.value("pseudocost_ratio", Khalil2016Obs::Features::pseudocost_ratio)
		.value("pseudocost_sum", Khalil2016Obs::Features::pseudocost_sum)
		.value("pseudocost_product", Khalil2016Obs::Features::pseudocost_product)
		.value("n_cutoff_up", Khalil2016Obs::Features::n_cutoff_up)
		.value("n_cutoff_down", Khalil2016Obs::Features::n_cutoff_down)
		.value("n_cutoff_up_ratio", Khalil2016Obs::Features::n_cutoff_up_ratio)
		.value("n_cutoff_down_ratio", Khalil2016Obs::Features::n_cutoff_down_ratio)
		.value("rows_dynamic_deg_mean", Khalil2016Obs::Features::rows_dynamic_deg_mean)
		.value("rows_dynamic_deg_stddev", Khalil2016Obs::Features::rows_dynamic_deg_stddev)
		.value("rows_dynamic_deg_min", Khalil2016Obs::Features::rows_dynamic_deg_min)
		.value("rows_dynamic_deg_max", Khalil2016Obs::Features::rows_dynamic_deg_max)
		.value("rows_dynamic_deg_mean_ratio", Khalil2016Obs::Features::rows_dynamic_deg_mean_ratio)
		.value("rows_dynamic_deg_min_ratio", Khalil2016Obs::Features::rows_dynamic_deg_min_ratio)
		.value("rows_dynamic_deg_max_ratio", Khalil2016Obs::Features::rows_dynamic_deg_max_ratio)
		.value("coef_pos_rhs_ratio_min", Khalil2016Obs::Features::coef_pos_rhs_ratio_min)
		.value("coef_pos_rhs_ratio_max", Khalil2016Obs::Features::coef_pos_rhs_ratio_max)
		.value("coef_neg_rhs_ratio_min", Khalil2016Obs::Features::coef_neg_rhs_ratio_min)
		.value("coef_neg_rhs_ratio_max", Khalil2016Obs::Features::coef_neg_rhs_ratio_max)
		.value("pos_coef_pos_coef_ratio_min", Khalil2016Obs::Features::pos_coef_pos_coef_ratio_min)
		.value("pos_coef_pos_coef_ratio_max", Khalil2016Obs::Features::pos_coef_pos_coef_ratio_max)
		.value("pos_coef_neg_coef_ratio_min", Khalil2016Obs::Features::pos_coef_neg_coef_ratio_min)
		.value("pos_coef_neg_coef_ratio_max", Khalil2016Obs::Features::pos_coef_neg_coef_ratio_max)
		.value("neg_coef_pos_coef_ratio_min", Khalil2016Obs::Features::neg_coef_pos_coef_ratio_min)
		.value("neg_coef_pos_coef_ratio_max", Khalil2016Obs::Features::neg_coef_pos_coef_ratio_max)
		.value("neg_coef_neg_coef_ratio_min", Khalil2016Obs::Features::neg_coef_neg_coef_ratio_min)
		.value("neg_coef_neg_coef_ratio_max", Khalil2016Obs::Features::neg_coef_neg_coef_ratio_max)
		.value("active_coef_weight1_count", Khalil2016Obs::Features::active_coef_weight1_count)
		.value("active_coef_weight1_sum", Khalil2016Obs::Features::active_coef_weight1_sum)
		.value("active_coef_weight1_mean", Khalil2016Obs::Features::active_coef_weight1_mean)
		.value("active_coef_weight1_stddev", Khalil2016Obs::Features::active_coef_weight1_stddev)
		.value("active_coef_weight1_min", Khalil2016Obs::Features::active_coef_weight1_min)
		.value("active_coef_weight1_max", Khalil2016Obs::Features::active_coef_weight1_max)
		.value("active_coef_weight2_count", Khalil2016Obs::Features::active_coef_weight2_count)
		.value("active_coef_weight2_sum", Khalil2016Obs::Features::active_coef_weight2_sum)
		.value("active_coef_weight2_mean", Khalil2016Obs::Features::active_coef_weight2_mean)
		.value("active_coef_weight2_stddev", Khalil2016Obs::Features::active_coef_weight2_stddev)
		.value("active_coef_weight2_min", Khalil2016Obs::Features::active_coef_weight2_min)
		.value("active_coef_weight2_max", Khalil2016Obs::Features::active_coef_weight2_max)
		.value("active_coef_weight3_count", Khalil2016Obs::Features::active_coef_weight3_count)
		.value("active_coef_weight3_sum", Khalil2016Obs::Features::active_coef_weight3_sum)
		.value("active_coef_weight3_mean", Khalil2016Obs::Features::active_coef_weight3_mean)
		.value("active_coef_weight3_stddev", Khalil2016Obs::Features::active_coef_weight3_stddev)
		.value("active_coef_weight3_min", Khalil2016Obs::Features::active_coef_weight3_min)
		.value("active_coef_weight3_max", Khalil2016Obs::Features::active_coef_weight3_max)
		.value("active_coef_weight4_count", Khalil2016Obs::Features::active_coef_weight4_count)
		.value("active_coef_weight4_sum", Khalil2016Obs::Features::active_coef_weight4_sum)
		.value("active_coef_weight4_mean", Khalil2016Obs::Features::active_coef_weight4_mean)
		.value("active_coef_weight4_stddev", Khalil2016Obs::Features::active_coef_weight4_stddev)
		.value("active_coef_weight4_min", Khalil2016Obs::Features::active_coef_weight4_min)
		.value("active_coef_weight4_max", Khalil2016Obs::Features::active_coef_weight4_max);

	auto khalil2016 = py::class_<Khalil2016>(m, "Khalil2016", R"(
		Branching candidates features from Khalil et al. (2016).

		This observation function extract structured :py:class:`Khalil2016Obs`.
	)");
	khalil2016.def(py::init<>());
	def_before_reset(khalil2016, R"(Reset static features cache.)");
	def_extract(khalil2016, "Extract the observation matrix.");
}

}  // namespace ecole::observation
