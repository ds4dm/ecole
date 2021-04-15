#include <cmath>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>

#include <nonstd/span.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include <xtensor/xfixed.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/khalil-2016.hpp"
#include "ecole/scip/col.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/row.hpp"

namespace ecole::observation {

namespace {

namespace views = ranges::views;

using Features = Khalil2016Obs::Features;
using value_type = decltype(Khalil2016Obs::features)::value_type;

/*************************
 *  Algorithm functions  *
 *************************/

/**
 * Square of a number.
 */
template <typename T> auto square(T x) noexcept -> T {
	return x * x;
}

/**
 * Floating points division that return 0 when the denominator is 0.
 * Also ensures that it isn't accidentally called with integers which would lead to euclidian
 * division.
 */
template <typename T> auto safe_div(T x, T y) noexcept -> T {
	static_assert(std::is_floating_point_v<T>, "Inputs are not decimals");
	return y != 0. ? x / y : 0.;
}

/**
 * Compute the sum of and count of elements.
 *
 * @param range The container to iteratre over.
 * @param transform A callable to apply on every element of the container.
 * @param filter A callable to filter in elements (after transformation).
 */
template <typename Range> auto count_sum(Range range) noexcept {
	auto sum = ranges::range_value_t<Range>{0};
	auto count = std::size_t{0};

	for (auto const element : range) {
		count++;
		sum += element;
	}
	return std::pair{count, sum};
}

struct StatsFeatures {
	value_type count = 0.;
	value_type sum = 0.;
	value_type mean = 0.;
	value_type stddev = 0.;
	value_type min = 0.;
	value_type max = 0.;
};

template <typename Range> auto compute_stats(Range range) noexcept -> StatsFeatures {
	auto const [count, sum] = count_sum(range);

	// We can assume count to be always positive after this point and that the (filtered) iteration
	// will contain at least one element.
	if (count == 0) {
		return {};
	}

	auto const mean = static_cast<value_type>(sum) / static_cast<value_type>(count);
	auto stddev = value_type{0.};
	auto min = std::numeric_limits<ranges::range_value_t<Range>>::max();
	auto max = std::numeric_limits<ranges::range_value_t<Range>>::min();

	for (auto const element : range) {
		min = std::min(min, element);
		max = std::max(max, element);
		stddev += square(static_cast<value_type>(element) - mean);
	}
	stddev = std::sqrt(stddev / static_cast<value_type>(count));

	return {
		static_cast<value_type>(count),
		static_cast<value_type>(sum),
		mean,
		stddev,
		static_cast<value_type>(min),
		static_cast<value_type>(max)};
}

/**
 * Return the sum of positive numbers and the sum of negative numbers in a range.
 */
template <typename Range> auto sum_positive_negative(Range range) noexcept {
	auto positive_sum = ranges::range_value_t<Range>{0};
	auto negative_sum = ranges::range_value_t<Range>{0};
	for (auto const val : range) {
		if (val > 0) {
			positive_sum += val;
		} else {
			negative_sum += val;
		}
	}
	return std::pair{positive_sum, negative_sum};
}

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

/******************************************
 *  Static features extraction functions  *
 ******************************************/

/* Feature as defined and split in table 1 of the paper Khalil et al. "Learning to Branch in Mixed
 * Integer Programming" Thirtieth AAAI Conference on Artificial Intelligence. 2016.
 *
 * https://web.archive.org/web/20200812151256/https://www.cc.gatech.edu/~lsong/papers/KhaLebSonNemDil16.pdf
 */

/**
 * Objective function coeffs.
 *
 * Value of the coefficient (raw, positive only, negative only).
 */
template <typename Tensor> void set_objective_function_coefficient(Tensor&& out, SCIP_COL* const col) noexcept {
	auto const obj = SCIPcolGetObj(col);
	out[idx(Features::obj_coef)] = obj;
	out[idx(Features::obj_coef_pos_part)] = std::max(obj, 0.);
	out[idx(Features::obj_coef_neg_part)] = std::min(obj, 0.);
}

/**
 * Num. constraints.
 *
 * Number of constraints that the variable participates in (with a non-zero coefficient).
 */
template <typename Tensor> void set_number_constraints(Tensor&& out, SCIP_COL* const col) noexcept {
	out[idx(Features::n_rows)] = static_cast<value_type>(SCIPcolGetNNonz(col));
}

/**
 * Stats. for constraint degrees.
 *
 * The degree of a constraint is the number of variables that participate in it.
 * A variable may participate in multiple constraints, and statistics over those constraints'
 * degrees are used.
 * The constraint degree is computed on the root LP (mean, stdev., min, max)
 */
template <typename Tensor>
void set_static_stats_for_constraint_degree(Tensor&& out, nonstd::span<SCIP_ROW*> const rows) noexcept {
	auto row_get_nnz = [](auto const row) { return static_cast<std::size_t>(SCIProwGetNNonz(row)); };
	auto const stats = compute_stats(rows | ranges::views::transform(row_get_nnz));
	out[idx(Features::rows_deg_mean)] = stats.mean;
	out[idx(Features::rows_deg_stddev)] = stats.stddev;
	out[idx(Features::rows_deg_min)] = stats.min;
	out[idx(Features::rows_deg_max)] = stats.max;
}

/**
 * Stats. for constraint coeffs.
 *
 * A variable's positive coefficients in the constraints it participates in
 * (count, mean, stdev., min, max).
 */
template <typename Tensor>
void set_stats_for_constraint_positive_coefficients(Tensor&& out, nonstd::span<SCIP_Real> const coefficients) noexcept {
	auto const stats = compute_stats(coefficients | views::filter([](auto x) { return x > 0.; }));
	out[idx(Features::rows_pos_coefs_count)] = stats.count;
	out[idx(Features::rows_pos_coefs_mean)] = stats.mean;
	out[idx(Features::rows_pos_coefs_stddev)] = stats.stddev;
	out[idx(Features::rows_pos_coefs_min)] = stats.min;
	out[idx(Features::rows_pos_coefs_max)] = stats.max;
}

/**
 * Stats. for constraint coeffs.
 *
 * A variable's negative coefficients in the constraints it participates in
 * (count, mean, stdev., min, max).
 */
template <typename Tensor>
void set_stats_for_constraint_negative_coefficients(Tensor&& out, nonstd::span<SCIP_Real> const coefficients) noexcept {
	auto const stats = compute_stats(coefficients | views::filter([](auto x) { return x < 0.; }));
	out[idx(Features::rows_neg_coefs_count)] = stats.count;
	out[idx(Features::rows_neg_coefs_mean)] = stats.mean;
	out[idx(Features::rows_neg_coefs_stddev)] = stats.stddev;
	out[idx(Features::rows_neg_coefs_min)] = stats.min;
	out[idx(Features::rows_neg_coefs_max)] = stats.max;
}

/**
 * Extract the static features for a single LP columns.
 */
template <typename Tensor> void set_static_features(Tensor&& out, SCIP_COL* const col) {
	auto const rows = scip::get_rows(col);
	auto const coefficients = scip::get_vals(col);

	set_objective_function_coefficient(out, col);
	set_number_constraints(out, col);
	set_static_stats_for_constraint_degree(out, rows);
	set_stats_for_constraint_positive_coefficients(out, coefficients);
	set_stats_for_constraint_negative_coefficients(out, coefficients);
}

/**
 * Extract the static features for all LP columns in a Model.
 */
auto extract_static_features(scip::Model& model) {
	auto const columns = model.lp_columns();
	xt::xtensor<value_type, 2> static_features{{columns.size(), Khalil2016Obs::n_static_features}, 0.};

	auto const n_columns = columns.size();
	for (std::size_t i = 0; i < n_columns; ++i) {
		set_static_features(xt::row(static_features, static_cast<std::ptrdiff_t>(i)), columns[i]);
	}

	return static_features;
}

/*******************************************
 *  Dynamic features extraction functions  *
 *******************************************/

/* Feature as defined and split in table 1 of the paper Khalil et al. "Learning to Branch in Mixed
 * Integer Programming" Thirtieth AAAI Conference on Artificial Intelligence. 2016.
 *
 * https://web.archive.org/web/20200812151256/https://www.cc.gatech.edu/~lsong/papers/KhaLebSonNemDil16.pdf
 */

/**
 * Slack, ceil distances, and Pseudocosts.
 *
 * This function combines two feature sets from Khalil et al.
 *
 * Slack and ceil distance:
 *     min{xij−floor(xij),ceil(xij) −xij} and ceil(xij) −xij
 *
 * Pseudocosts:
 *     Upwards and downwards values, and their corresponding ratio, sum and product, weighted by the
 *     fractionality of xj.
 */
template <typename Tensor>
void set_slack_ceil_and_pseudocosts(Tensor&& out, SCIP* const scip, SCIP_VAR* const var, SCIP_COL* const col) noexcept {
	auto const solval = SCIPcolGetPrimsol(col);
	auto const floor_distance = SCIPfeasFrac(scip, solval);
	auto const ceil_distance = 1. - floor_distance;
	auto const weighted_pseudocost_up = ceil_distance * SCIPgetVarPseudocost(scip, var, SCIP_BRANCHDIR_UPWARDS);
	auto const weighted_pseudocost_down = floor_distance * SCIPgetVarPseudocost(scip, var, SCIP_BRANCHDIR_DOWNWARDS);
	auto constexpr epsilon = 1e-5;
	auto const wpu_approx = std::max(weighted_pseudocost_up, epsilon);
	auto const wpd_approx = std::max(weighted_pseudocost_down, epsilon);
	auto const weighted_pseudocost_ratio = safe_div(std::min(wpu_approx, wpd_approx), std::max(wpu_approx, wpd_approx));
	out[idx(Features::slack)] = std::min(floor_distance, ceil_distance);
	out[idx(Features::ceil_dist)] = ceil_distance;
	out[idx(Features::pseudocost_up)] = weighted_pseudocost_up;
	out[idx(Features::pseudocost_down)] = weighted_pseudocost_down;
	out[idx(Features::pseudocost_ratio)] = weighted_pseudocost_ratio;
	out[idx(Features::pseudocost_sum)] = weighted_pseudocost_up + weighted_pseudocost_down;
	out[idx(Features::pseudocost_product)] = weighted_pseudocost_up * weighted_pseudocost_down;
}

/**
 * Infeasibility statistics.
 *
 * Number and fraction of nodes for which applying SB to variable xj led to one (two) infeasible
 * children (during data collection).
 *
 * N.B. replaced by left, right infeasibility.
 */
template <typename Tensor> void set_infeasibility_statistics(Tensor&& out, SCIP_VAR* const var) noexcept {
	auto const n_infeasibles_up = SCIPvarGetCutoffSum(var, SCIP_BRANCHDIR_UPWARDS);
	auto const n_infeasibles_down = SCIPvarGetCutoffSum(var, SCIP_BRANCHDIR_DOWNWARDS);
	auto const n_branchings_up = static_cast<value_type>(SCIPvarGetNBranchings(var, SCIP_BRANCHDIR_UPWARDS));
	auto const n_branchings_down = static_cast<value_type>(SCIPvarGetNBranchings(var, SCIP_BRANCHDIR_DOWNWARDS));
	out[idx(Features::n_cutoff_up)] = n_infeasibles_up;
	out[idx(Features::n_cutoff_down)] = n_infeasibles_down;
	out[idx(Features::n_cutoff_up_ratio)] = safe_div(n_infeasibles_up, n_branchings_up);
	out[idx(Features::n_cutoff_down_ratio)] = safe_div(n_infeasibles_down, n_branchings_down);
}

/**
 * Stats. for constraint degrees.
 *
 * A dynamic variant of the static version above. Here, the constraint degrees are on the current
 * node's LP.
 * The ratios of the static mean, maximum and minimum to their dynamic counterparts are also
 * features.
 *
 * The precomputed static features given as input parameters are wrapped in their strong type to
 * avoid passing the wrong ones.
 */
template <typename Tensor>
void set_dynamic_stats_for_constraint_degree(Tensor&& out, nonstd::span<SCIP_ROW*> const rows) noexcept {
	auto row_get_lp_nnz = [](auto const row) { return static_cast<std::size_t>(SCIProwGetNLPNonz(row)); };
	auto const stats = compute_stats(rows | views::transform(row_get_lp_nnz));
	auto const root_deg_mean = out[idx(Features::rows_deg_mean)];
	auto const root_deg_min = out[idx(Features::rows_deg_min)];
	auto const root_deg_max = out[idx(Features::rows_deg_max)];
	out[idx(Features::rows_dynamic_deg_mean)] = stats.mean;
	out[idx(Features::rows_dynamic_deg_stddev)] = stats.stddev;
	out[idx(Features::rows_dynamic_deg_min)] = stats.min;
	out[idx(Features::rows_dynamic_deg_max)] = stats.max;
	out[idx(Features::rows_dynamic_deg_mean_ratio)] = safe_div(stats.mean, root_deg_mean + stats.mean);
	out[idx(Features::rows_dynamic_deg_min_ratio)] = safe_div(stats.min, root_deg_min + stats.min);
	out[idx(Features::rows_dynamic_deg_max_ratio)] = safe_div(stats.max, root_deg_max + stats.max);
}

/**
 * Min/max for ratios of constraint coeffs. to RHS.
 *
 * Minimum and maximum ratios across positive and negative right-hand-sides (RHS).
 */
template <typename Tensor>
void set_min_max_for_ratios_constraint_coeffs_rhs(
	Tensor&& out,
	SCIP* const scip,
	nonstd::span<SCIP_ROW*> const rows,
	nonstd::span<SCIP_Real> const coefficients) noexcept {

	value_type positive_rhs_ratio_max = -1.;
	value_type positive_rhs_ratio_min = 1.;
	value_type negative_rhs_ratio_max = -1.;
	value_type negative_rhs_ratio_min = 1.;

	auto rhs_ratio_updates = [&](auto const coef, auto const rhs) {
		auto const ratio_val = safe_div(coef, std::abs(coef) + std::abs(rhs));
		if (rhs >= 0) {
			positive_rhs_ratio_max = std::max(positive_rhs_ratio_max, ratio_val);
			positive_rhs_ratio_min = std::min(positive_rhs_ratio_min, ratio_val);
		} else {
			negative_rhs_ratio_max = std::max(negative_rhs_ratio_max, ratio_val);
			negative_rhs_ratio_min = std::min(negative_rhs_ratio_min, ratio_val);
		}
	};

	for (auto const [row, coef] : views::zip(rows, coefficients)) {
		if (auto const rhs = SCIProwGetRhs(row); !SCIPisInfinity(scip, std::abs(rhs))) {
			rhs_ratio_updates(coef, rhs);
		}
		if (auto const lhs = SCIProwGetLhs(row); !SCIPisInfinity(scip, std::abs(lhs))) {
			// lhs constraints are multiply by -1 to be considered as rhs constraints.
			rhs_ratio_updates(-coef, -lhs);
		}
	}

	out[idx(Features::coef_pos_rhs_ratio_min)] = positive_rhs_ratio_min;
	out[idx(Features::coef_pos_rhs_ratio_max)] = positive_rhs_ratio_max;
	out[idx(Features::coef_neg_rhs_ratio_min)] = negative_rhs_ratio_min;
	out[idx(Features::coef_neg_rhs_ratio_max)] = negative_rhs_ratio_max;
}

/**
 * Min/max for one-to-all coefficient ratios.
 *
 * The statistics are over the ratios of a variable's coefficient, to the sum over all other
 * variables' coefficients, for a given constraint.
 * Four versions of these ratios are considered: positive (negative) coefficient to sum of
 * positive (negative) coefficients.
 */
template <typename Tensor>
void set_min_max_for_one_to_all_coefficient_ratios(
	Tensor&& out,
	nonstd::span<SCIP_ROW*> const rows,
	nonstd::span<SCIP_Real> const coefficients) noexcept {

	value_type positive_positive_ratio_max = 0;
	value_type positive_positive_ratio_min = 1;
	value_type positive_negative_ratio_max = 0;
	value_type positive_negative_ratio_min = 1;
	value_type negative_positive_ratio_max = 0;
	value_type negative_positive_ratio_min = 1;
	value_type negative_negative_ratio_max = 0;
	value_type negative_negative_ratio_min = 1;

	for (auto const [row, coef] : views::zip(rows, coefficients)) {
		auto const [positive_coeficients_sum, negative_coeficients_sum] = sum_positive_negative(scip::get_vals(row));
		if (coef > 0) {
			auto const positive_ratio = coef / positive_coeficients_sum;
			auto const negative_ratio = coef / (coef - negative_coeficients_sum);
			positive_positive_ratio_max = std::max(positive_positive_ratio_max, positive_ratio);
			positive_positive_ratio_min = std::min(positive_positive_ratio_min, positive_ratio);
			positive_negative_ratio_max = std::max(positive_negative_ratio_max, negative_ratio);
			positive_negative_ratio_min = std::min(positive_negative_ratio_min, negative_ratio);
		} else if (coef < 0) {
			auto const positive_ratio = coef / (coef - positive_coeficients_sum);
			auto const negative_ratio = coef / negative_coeficients_sum;
			negative_positive_ratio_max = std::max(negative_positive_ratio_max, positive_ratio);
			negative_positive_ratio_min = std::min(negative_positive_ratio_min, positive_ratio);
			negative_negative_ratio_max = std::max(negative_negative_ratio_max, negative_ratio);
			negative_negative_ratio_min = std::min(negative_negative_ratio_min, negative_ratio);
		}
	}

	out[idx(Features::pos_coef_pos_coef_ratio_min)] = positive_positive_ratio_min;
	out[idx(Features::pos_coef_pos_coef_ratio_max)] = positive_positive_ratio_max;
	out[idx(Features::pos_coef_neg_coef_ratio_min)] = positive_negative_ratio_min;
	out[idx(Features::pos_coef_neg_coef_ratio_max)] = positive_negative_ratio_max;
	out[idx(Features::neg_coef_pos_coef_ratio_min)] = negative_positive_ratio_min;
	out[idx(Features::neg_coef_pos_coef_ratio_max)] = negative_positive_ratio_max;
	out[idx(Features::neg_coef_neg_coef_ratio_min)] = negative_negative_ratio_min;
	out[idx(Features::neg_coef_neg_coef_ratio_max)] = negative_negative_ratio_max;
}

/**
 * Return if a row in the constraints is active in the LP.
 */
auto row_is_active(SCIP* const scip, SCIP_ROW* const row) noexcept -> bool {
	auto const activity = SCIPgetRowActivity(scip, row);
	auto const lhs = SCIProwGetLhs(row);
	auto const rhs = SCIProwGetRhs(row);
	return SCIPisEQ(scip, activity, rhs) || SCIPisEQ(scip, activity, lhs);
}

/**
 * Compute the weight necessary for the stats for active constraints coefficients.
 *
 * The four coefficients are
 *   - unit weight,
 *   - inverse of the sum of the coefficients of all variables in constraint,
 *   - inverse of the sum of the coefficients of only candidate variables in constraint
 *   - dual cost of the constraint.
 * They are computed for every row that is active, as defined by @ref row_is_active.
 * Weights for non activate rows are left as NaN and ununsed.
 * This is equivalent to an unsafe/unchecked masked tensor.
 */
auto stats_for_active_constraint_coefficients_weights(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
	auto const lp_rows = model.lp_rows();
	auto const branch_candidates = model.pseudo_branch_cands() | ranges::to<std::set>();

	/** Check if a column is a branching candidate. */
	auto is_candidate = [&branch_candidates](auto* col) { return branch_candidates.count(SCIPcolGetVar(col)) > 0; };

	/** Compute the sum of absolute values in a range. */
	auto sum_abs = [](auto range) {
		return ranges::accumulate(range | views::transform([](auto x) { return std::abs(x); }), 0.);
	};

	/** Compute the sum of absolute value of column coefficient if the column is a branching candidate. */
	auto sum_abs_if_candidate = [&is_candidate](auto const& row_cols, auto const& row_cols_vals) {
		auto sum = value_type{0.};
		for (auto const [col, val] : views::zip(row_cols, row_cols_vals)) {
			if (is_candidate(col)) {
				sum += std::abs(val);
			}
		}
		return sum;
	};

	/** Compute the inverse of a number or 1 if the number is zero. */
	auto safe_inv = [](auto const x) { return x != 0. ? 1. / x : 1.; };

	xt::xtensor<value_type, 2> weights{{lp_rows.size(), 4}, std::nan("")};
	auto* weights_iter = weights.begin();

	for (auto* const row : lp_rows) {
		if (row_is_active(scip, row)) {
			auto const row_cols_vals = scip::get_vals(row);
			*(weights_iter++) = 1.;
			*(weights_iter++) = safe_inv(sum_abs(row_cols_vals));
			*(weights_iter++) = safe_inv(sum_abs_if_candidate(scip::get_cols(row), row_cols_vals));
			*(weights_iter++) = std::abs(SCIProwGetDualsol(row));
		} else {
			weights_iter += 4;
		}
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(weights_iter == weights.cend());

	return weights;
}

/**
 * Stats. for active constraint coefficients.
 *
 * An active constraint at a node LP is one which is binding with equality at the optimum.
 * We consider 4 weighting schemes for an active constraint: unit weight, inverse of the sum of
 * the coefficients of all variables in constraint, inverse of the sum of the coefficients of only
 * candidate variables in constraint, dual cost of the constraint.
 * Given the absolute value of the coefficients of xj in the active constraints, we compute the
 * sum, mean, stdev., max. and min. of those values, for each of the weighting schemes. We also
 * compute the weighted number of active constraints that xj is in, with the same 4 weightings.
 */
template <typename Tensor>
void set_stats_for_active_constraint_coefficients(
	Tensor&& out,
	SCIP* const scip,
	nonstd::span<SCIP_ROW*> const rows,
	nonstd::span<SCIP_Real> const coefficients,
	xt::xtensor<value_type, 2> const& active_rows_weights) noexcept {

	auto weights_stats = std::array<StatsFeatures, 4>{};
	for (auto& stats : weights_stats) {
		stats.min = std::numeric_limits<decltype(stats.min)>::max();
		stats.max = std::numeric_limits<decltype(stats.max)>::min();
	}

	std::size_t n_active_rows = 0UL;
	for (auto const [row, coef] : views::zip(rows, coefficients)) {
		auto const row_lp_idx = SCIProwGetLPPos(row);

		if (row_is_active(scip, row)) {
			n_active_rows++;

			for (std::size_t weight_idx = 0; weight_idx < weights_stats.size(); ++weight_idx) {
				auto const weight = active_rows_weights(row_lp_idx, weight_idx);
				assert(!std::isnan(weight));  // If NaN likely hit a maked value
				auto const weighted_abs_coef = weight * std::abs(coef);

				auto& stats = weights_stats[weight_idx];
				stats.count += weight;
				stats.sum += weighted_abs_coef;
				stats.min = std::min(stats.min, weighted_abs_coef);
				stats.max = std::max(stats.max, weighted_abs_coef);
			}
		}
	}

	if (n_active_rows > 0) {
		for (auto& stats : weights_stats) {
			stats.mean = stats.sum / static_cast<value_type>(n_active_rows);
		}

		for (auto const [row, coef] : views::zip(rows, coefficients)) {
			auto const row_lp_idx = SCIProwGetLPPos(row);
			if (row_is_active(scip, row)) {
				for (std::size_t weight_idx = 0; weight_idx < weights_stats.size(); ++weight_idx) {
					auto const weight = active_rows_weights(row_lp_idx, weight_idx);
					assert(!std::isnan(weight));  // If NaN likely hit a maked value
					auto const weighted_abs_coef = weight * std::abs(coef);

					auto& stats = weights_stats[weight_idx];
					stats.stddev = square(weighted_abs_coef - stats.mean);
				}
			}
		}

		for (auto& stats : weights_stats) {
			stats.stddev = std::sqrt(stats.stddev / static_cast<value_type>(n_active_rows));
		}

	} else {
		for (auto& stats : weights_stats) {
			stats = {};
		}
	}

	out[idx(Features::active_coef_weight1_count)] = weights_stats[0].count;
	out[idx(Features::active_coef_weight1_sum)] = weights_stats[0].sum;
	out[idx(Features::active_coef_weight1_mean)] = weights_stats[0].mean;
	out[idx(Features::active_coef_weight1_stddev)] = weights_stats[0].stddev;
	out[idx(Features::active_coef_weight1_min)] = weights_stats[0].min;
	out[idx(Features::active_coef_weight1_max)] = weights_stats[0].max;
	out[idx(Features::active_coef_weight2_count)] = weights_stats[1].count;
	out[idx(Features::active_coef_weight2_sum)] = weights_stats[1].sum;
	out[idx(Features::active_coef_weight2_mean)] = weights_stats[1].mean;
	out[idx(Features::active_coef_weight2_stddev)] = weights_stats[1].stddev;
	out[idx(Features::active_coef_weight2_min)] = weights_stats[1].min;
	out[idx(Features::active_coef_weight2_max)] = weights_stats[1].max;
	out[idx(Features::active_coef_weight3_count)] = weights_stats[2].count;
	out[idx(Features::active_coef_weight3_sum)] = weights_stats[2].sum;
	out[idx(Features::active_coef_weight3_mean)] = weights_stats[2].mean;
	out[idx(Features::active_coef_weight3_stddev)] = weights_stats[2].stddev;
	out[idx(Features::active_coef_weight3_min)] = weights_stats[2].min;
	out[idx(Features::active_coef_weight3_max)] = weights_stats[2].max;
	out[idx(Features::active_coef_weight4_count)] = weights_stats[3].count;
	out[idx(Features::active_coef_weight4_sum)] = weights_stats[3].sum;
	out[idx(Features::active_coef_weight4_mean)] = weights_stats[3].mean;
	out[idx(Features::active_coef_weight4_stddev)] = weights_stats[3].stddev;
	out[idx(Features::active_coef_weight4_min)] = weights_stats[3].min;
	out[idx(Features::active_coef_weight4_max)] = weights_stats[3].max;
}

/**
 * Extract the dynamic features for a single branching candidate variable.
 *
 * The precomputed static features given as input parameters are wrapped in their strong type to
 * avoid passing the wrong ones.
 */
template <typename Tensor>
void set_dynamic_features(
	Tensor&& out,
	SCIP* const scip,
	SCIP_VAR* const var,
	xt::xtensor<value_type, 2> const& active_rows_weights) {
	auto* const col = SCIPvarGetCol(var);
	auto const rows = scip::get_rows(col);
	auto const coefficients = scip::get_vals(col);

	set_slack_ceil_and_pseudocosts(out, scip, var, col);
	set_infeasibility_statistics(out, var);
	set_dynamic_stats_for_constraint_degree(out, rows);
	set_min_max_for_ratios_constraint_coeffs_rhs(out, scip, rows, coefficients);
	set_min_max_for_one_to_all_coefficient_ratios(out, rows, coefficients);
	set_stats_for_active_constraint_coefficients(out, scip, rows, coefficients, active_rows_weights);
}

/**
 * Extract the static features already computed.
 *
 * The static features have been computed for all LP columns and stored in the order of `LPcolumns`.
 * We need to find the one associated with the given variable.
 */
template <typename Tensor>
void set_precomputed_static_features(
	Tensor&& out,
	SCIP_VAR* const var,
	xt::xtensor<value_type, 2> const& static_features) {

	auto const col_idx = static_cast<std::ptrdiff_t>(SCIPcolGetIndex(SCIPvarGetCol(var)));
	using namespace xt::placeholders;
	xt::view(out, xt::range(_, Khalil2016Obs::n_static_features)) = xt::row(static_features, col_idx);
}

/******************************
 *  Main extraction function  *
 ******************************/

auto extract_all_features(scip::Model& model, xt::xtensor<value_type, 2> const& static_features) {
	xt::xtensor<value_type, 2> observation{
		{model.pseudo_branch_cands().size(), Khalil2016Obs::n_features},
		std::nan(""),
	};

	auto* const scip = model.get_scip_ptr();
	auto const active_rows_weights = stats_for_active_constraint_coefficients_weights(model);

	auto const pseudo_branch_cands = model.pseudo_branch_cands();
	auto const n_pseudo_branch_cands = pseudo_branch_cands.size();
	for (std::size_t var_idx = 0; var_idx < n_pseudo_branch_cands; ++var_idx) {
		auto* const var = pseudo_branch_cands[var_idx];
		auto features = xt::row(observation, static_cast<std::ptrdiff_t>(var_idx));
		set_precomputed_static_features(features, var, static_features);
		set_dynamic_features(features, scip, var, active_rows_weights);
	}

	return observation;
}

auto is_on_root_node(scip::Model& model) -> bool {
	auto* const scip = model.get_scip_ptr();
	return SCIPgetCurrentNode(scip) == SCIPgetRootNode(scip);
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

void Khalil2016::before_reset(scip::Model& /* model */) {
	static_features = decltype(static_features){};
}

auto Khalil2016::extract(scip::Model& model, bool /* done */) -> std::optional<Khalil2016Obs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		if (is_on_root_node(model)) {
			static_features = extract_static_features(model);
		}
		return {{extract_all_features(model, static_features)}};
	}
	return {};
}

}  // namespace ecole::observation
