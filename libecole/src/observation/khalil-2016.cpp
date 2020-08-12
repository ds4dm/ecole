#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>

#include <nonstd/span.hpp>

#include "ecole/observation/khalil-2016.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

namespace {

using value_type = Khalil2016Obs::value_type;

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
	static_assert(std::is_floating_point<T>::value, "Inputs are not decimals");
	return y != 0. ? y / x : 0.;
}

/**
 * Wrap std::copy to use directly with range container.
 */
template <typename InputContainer, typename OutputIt>
auto copy(InputContainer const& container, OutputIt out_first) -> OutputIt {
	return std::copy(container.begin(), container.end(), out_first);
}

/**
 * Wrap std::accumulate (with binary operation) to use directly with a range container.
 */
template <typename InputContainer, typename T, typename BinaryOperation>
auto accumulate(InputContainer const& container, T init, BinaryOperation op) -> T {
	return std::accumulate(container.cbegin(), container.cend(), std::move(init), std::move(op));
}

/**
 * Sum of the absolute value of element in a range.
 */
template <typename InputContainer> auto sum_abs(InputContainer const& container) {
	return accumulate(
		container,
		typename InputContainer::value_type{0},
		[](auto const cumulate, auto const element) { return cumulate + std::abs(element); }  //
	);
}

/**
 * Identity function to use as unary operation.
 */
auto const identity = [](auto const x) { return x; };

/**
 * Greater than filtering function object.
 */
template <typename T> auto greater_than(T other) noexcept {
	struct greater_than_func {
		T other = 0.;
		bool operator()(T elem) const noexcept { return elem > other; }
	};

	return greater_than_func{other};
}

/**
 * Lesser than filtering function object.
 */
template <typename T> auto lesser_than(T other) noexcept {
	struct lesser_than_func {
		T other = 0.;
		bool operator()(T elem) const noexcept { return elem < other; }
	};

	return lesser_than_func{other};
}

/**
 * Compute the sum of and count of elements.
 *
 * @param range The container to iteratre over.
 * @param transform A callable to apply on every element of the container.
 * @param filter A callable to filter in elements (after transformation).
 */
template <typename Range, typename TransformFunc, typename FilterFunc>
auto count_sum(Range const& range, TransformFunc transform, FilterFunc filter) noexcept
	-> std::pair<value_type, value_type> {
	using transformed_type = decltype(transform(std::declval<typename Range::value_type>()));

	transformed_type sum_val = 0;
	std::size_t count_val = 0UL;

	for (auto const element : range) {
		auto const value = transform(element);
		if (filter(value)) {
			count_val++;
			sum_val += value;
		}
	}
	return {count_val, sum_val};
}

struct StatsFeatures {
	value_type count = 0.;
	value_type sum = 0.;
	value_type mean = 0.;
	value_type variance = 0.;
	value_type min = 0.;
	value_type max = 0.;
};

template <typename Range, typename TransformFunc, typename FilterFunc>
auto compute_stats(Range const& range, TransformFunc transform, FilterFunc filter) noexcept
	-> StatsFeatures {
	using transformed_type = decltype(transform(std::declval<typename Range::value_type>()));

	value_type count = 0.;
	value_type sum = 0.;
	std::tie(count, sum) = count_sum(range, transform, filter);

	// We can assume count to be always positive after this point and that the (filtered) iteration
	// will contain at least one element.
	if (count == 0.) {
		return {};
	}

	value_type const mean = sum / count;
	value_type variance = 0.;
	auto min = std::numeric_limits<transformed_type>::max();
	auto max = std::numeric_limits<transformed_type>::min();

	for (auto const range_value : range) {
		auto const value = transform(range_value);
		if (filter(value)) {
			min = std::min(min, value);
			max = std::max(max, value);
			variance += square(static_cast<value_type>(value) - mean);
		}
	}
	// FIXME should it be std deviation ?
	variance /= count;

	return {count, sum, mean, variance, static_cast<value_type>(min), static_cast<value_type>(max)};
}

/**
 * Return the sum of positive numbers and the sum of negative numbers in a range.
 */
template <typename Range> auto sum_positive_negative(Range const& range) noexcept {
	using sum_type = typename Range::value_type;
	sum_type positive_sum = 0;
	sum_type negative_sum = 0;
	for (auto const val : range) {
		if (val > 0) {
			positive_sum += val;
		} else {
			negative_sum += val;
		}
	}
	return std::make_pair(positive_sum, negative_sum);
};

/*****************************
 *  Scip wrapping functions  *
 *****************************/

auto scip_col_get_rows(scip::Col* const col) noexcept -> nonstd::span<scip::Row*> {
	auto const n_rows = SCIPcolGetNNonz(col);
	return {SCIPcolGetRows(col), static_cast<std::size_t>(n_rows)};
}

auto scip_col_get_vals(scip::Col* const col) noexcept -> nonstd::span<scip::real> {
	auto const n_rows = SCIPcolGetNNonz(col);
	return {SCIPcolGetVals(col), static_cast<std::size_t>(n_rows)};
}

auto scip_row_get_cols(scip::Row* const row) noexcept -> nonstd::span<scip::Col*> {
	auto const n_cols = SCIProwGetNNonz(row);
	return {SCIProwGetCols(row), static_cast<std::size_t>(n_cols)};
}

auto scip_row_get_vals(scip::Row* const row) noexcept -> nonstd::span<scip::real> {
	auto const n_cols = SCIProwGetNNonz(row);
	return {SCIProwGetVals(row), static_cast<std::size_t>(n_cols)};
}

/***********************************
 *  Features extraction functions  *
 ***********************************/

/* Feature as defined and split in table 1 of the paper Khalil et al. "Learning to Branch in Mixed
 * Integer Programming" Thirtieth AAAI Conference on Artificial Intelligence. 2016.
 *
 * https://web.archive.org/web/20200812151256/https://www.cc.gatech.edu/~lsong/papers/KhaLebSonNemDil16.pdf
 */

template <std::size_t N> using Features = std::array<value_type, N>;

/**
 * Objective function coeffs.
 *
 * Value of the coefficient (raw, positive only, negative only).
 */
auto objective_function_coefficient(scip::Col* const col) noexcept -> Features<3> {
	auto const obj = SCIPcolGetObj(col);
	return {obj, std::max(obj, 0.), std::min(obj, 0.)};
}

/**
 * Num. constraints.
 *
 * Number of constraints that the variable participates in (with a non-zero coefficient).
 */
auto number_constraints(scip::Col* const col) noexcept -> Features<1> {
	return {static_cast<value_type>(SCIPcolGetNNonz(col))};
}

/**
 * Stats. for constraint degrees helper function for caching.
 */
auto static_stats_for_constraint_degree_stats(nonstd::span<scip::Row*> const rows) noexcept
	-> StatsFeatures {
	auto transform = [](auto const row) { return static_cast<std::size_t>(SCIProwGetNNonz(row)); };
	auto filter = [](auto const /* degree */) { return true; };
	return compute_stats(rows, transform, filter);
}

/**
 * Stats. for constraint degrees.
 *
 * The degree of a constraint is the number of variables that participate in it.
 * A variable may participate in multiple constraints, and statistics over those constraints'
 * degrees are used.
 * The constraint degree is computed on the root LP (mean, stdev., min, max)
 *
 * @param root_stats the output of @ref static_stats_for_constraint_degree_stats.
 *        It is extracted in another function because the result is reused elsewhere.
 */
auto static_stats_for_constraint_degree(StatsFeatures const& root_stats) noexcept -> Features<4> {
	return {root_stats.mean, root_stats.variance, root_stats.min, root_stats.max};
}

/**
 * Stats. for constraint coeffs.
 *
 * A variable's positive coefficients in the constraints it participates in
 * (count, mean, stdev., min, max).
 */
auto stats_for_constraint_positive_coefficients(
	nonstd::span<scip::real> const coefficients) noexcept -> Features<5> {
	auto const stats = compute_stats(coefficients, identity, greater_than(0.));
	return {stats.count, stats.mean, stats.variance, stats.min, stats.max};
}

/**
 * Stats. for constraint coeffs.
 *
 * A variable's negative coefficients in the constraints it participates in
 * (count, mean, stdev., min, max).
 */
auto stats_for_constraint_negative_coefficients(
	nonstd::span<scip::real> const coefficients) noexcept -> Features<5> {
	auto const stats = compute_stats(coefficients, identity, lesser_than(0.));
	return {stats.count, stats.mean, stats.variance, stats.min, stats.max};
}

/**
 * Slack and ceil distances.
 *
 * min{xij−floor(xij),ceil(xij) −xij} and ceil(xij) −xij
 */
auto slack_ceil_distances(Scip* const scip, scip::Col* const col) noexcept -> Features<2> {
	auto const solval = SCIPcolGetPrimsol(col);
	auto const floor_distance = SCIPfeasFrac(scip, solval);
	auto const ceil_distance = 1. - floor_distance;
	return {std::min(floor_distance, ceil_distance), ceil_distance};
}

/**
 * Pseudocosts.
 *
 * Upwards and downwards values, and their corresponding ratio, sum and product, weighted by the
 * fractionality of xj.
 */
auto pseudocosts(Scip* const scip, scip::Var* const var) noexcept -> Features<5> {
	// FIXME how do we compute the ratio ? What about the weighting ?
	auto const pseudocost_up = SCIPgetVarPseudocost(scip, var, SCIP_BRANCHDIR_UPWARDS);
	auto const pseudocost_down = SCIPgetVarPseudocost(scip, var, SCIP_BRANCHDIR_DOWNWARDS);
	auto const pseudocost_sum = pseudocost_up + pseudocost_down;
	auto const pseudocost_ratio = pseudocost_up != 0. ? pseudocost_up / pseudocost_sum : 0.;
	auto const pseudocost_prod = pseudocost_up * pseudocost_down;
	return {pseudocost_up, pseudocost_down, pseudocost_ratio, pseudocost_sum, pseudocost_prod};
}

/**
 * Infeasibility statistics.
 *
 * Number and fraction of nodes for which applying SB to variable xj led to one (two) infeasible
 * children (during data collection).
 */
auto infeasibility_statistics(scip::Var* const var) noexcept -> Features<4> {
	// FIXME N.B. replaced by left, right infeasibility
	auto const n_infeasibles_up = SCIPvarGetCutoffSum(var, SCIP_BRANCHDIR_UPWARDS);
	auto const n_infeasibles_down = SCIPvarGetCutoffSum(var, SCIP_BRANCHDIR_DOWNWARDS);
	auto const n_branchings_up = SCIPvarGetNBranchings(var, SCIP_BRANCHDIR_UPWARDS);
	auto const n_branchings_down = SCIPvarGetNBranchings(var, SCIP_BRANCHDIR_DOWNWARDS);
	return {
		n_infeasibles_up,
		n_infeasibles_down,
		safe_div(n_infeasibles_up, static_cast<value_type>(n_branchings_up)),
		safe_div(n_infeasibles_down, static_cast<value_type>(n_branchings_down)),
	};
}

/**
 * Stats. for constraint degrees.
 *
 * A dynamic variant of the static version above. Here, the constraint degrees are on the current
 * node's LP.
 * The ratios of the static mean, maximum and minimum to their dynamic counterparts are also
 * features.
 */
auto dynamic_stats_for_constraint_degree(
	nonstd::span<scip::Row*> const rows,
	StatsFeatures const& root_stats) noexcept -> Features<7> {
	auto transform = [](auto const row) { return static_cast<std::size_t>(SCIProwGetNLPNonz(row)); };
	auto filter = [](auto const /* degree */) { return true; };
	auto const stats = compute_stats(rows, transform, filter);
	return {
		stats.mean,
		stats.variance,
		stats.min,
		stats.max,
		safe_div(stats.mean, root_stats.mean + stats.mean),
		safe_div(stats.min, root_stats.min + stats.min),
		safe_div(stats.max, root_stats.max + stats.max),
	};
}

/**
 * Min/max for ratios of constraint coeffs. to RHS.
 *
 * Minimum and maximum ratios across positive and negative right-hand-sides (RHS).
 */
auto min_max_for_ratios_constraint_coeffs_rhs(
	Scip* const scip,
	nonstd::span<scip::Row*> const rows,
	nonstd::span<scip::real> const coefficients) noexcept -> Features<4> {

	value_type positive_rhs_ratio_max = -1.;
	value_type positive_rhs_ratio_min = 1.;
	value_type negative_rhs_ratio_max = -1.;
	value_type negative_rhs_ratio_min = 1.;

	auto rhs_ratio_updates = [&](auto const coef, auto const rhs) {
		if (!SCIPisInfinity(scip, std::abs(rhs))) {
			auto const ratio_val = safe_div(coef, std::abs(coef) + std::abs(rhs));
			if (rhs >= 0) {
				positive_rhs_ratio_max = std::max(positive_rhs_ratio_max, ratio_val);
				positive_rhs_ratio_min = std::min(positive_rhs_ratio_min, ratio_val);
			} else {
				negative_rhs_ratio_max = std::max(negative_rhs_ratio_max, ratio_val);
				negative_rhs_ratio_min = std::min(negative_rhs_ratio_min, ratio_val);
			}
		}
	};

	auto const size = rows.size();
	for (std::size_t i = 0; i < size; ++i) {
		auto const coef = coefficients[i];
		rhs_ratio_updates(coef, SCIProwGetRhs(rows[i]));
		// FIXME Should it not be -coeff given that we multiply by -1 to get a lhs into a rhs
		rhs_ratio_updates(coef, -SCIProwGetLhs(rows[i]));
	}

	return {
		positive_rhs_ratio_max,
		positive_rhs_ratio_min,
		negative_rhs_ratio_max,
		negative_rhs_ratio_min,
	};
}

/**
 * Min/max for one-to-all coefficient ratios.
 *
 * The statistics are over the ratios of a variable's coefficient, to the sum over all other
 * variables' coefficients, for a given constraint.
 * Four versions of these ratios are considered: positive (negative) coefficient to sum of positive
 * (negative) coefficients.
 */
auto min_max_for_one_to_all_coefficient_ratios(
	nonstd::span<scip::Row*> const rows,
	nonstd::span<scip::real> const coefficients) noexcept -> Features<8> {

	value_type positive_positive_ratio_max = 0;
	value_type positive_positive_ratio_min = 1;
	value_type positive_negative_ratio_max = 0;
	value_type positive_negative_ratio_min = 1;
	value_type negative_positive_ratio_max = 0;
	value_type negative_positive_ratio_min = 1;
	value_type negative_negative_ratio_max = 0;
	value_type negative_negative_ratio_min = 1;

	auto const size = rows.size();
	for (std::size_t i = 0; i < size; ++i) {
		auto const sums = sum_positive_negative(scip_row_get_vals(rows[i]));
		value_type const positive_coeficients_sum = sums.first;
		value_type const negative_coeficients_sum = sums.second;
		auto const coef = coefficients[i];
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

	return {
		positive_positive_ratio_max,
		positive_positive_ratio_min,
		positive_negative_ratio_max,
		positive_negative_ratio_min,
		negative_positive_ratio_max,
		negative_positive_ratio_min,
		negative_negative_ratio_max,
		negative_negative_ratio_min,
	};
}

/**
 * Return if a row in the constraints is active in the LP.
 */
auto row_is_active(Scip* const scip, scip::Row* const row) noexcept -> bool {
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
auto stats_for_active_constraint_coefficients_weights(scip::Model const& model) {
	auto const scip = model.get_scip_ptr();
	auto const lp_rows = model.lp_rows();
	auto const branch_candidates_span = model.pseudo_branch_cands();
	std::set<scip::Var*> const branch_candidates{
		branch_candidates_span.begin(), branch_candidates_span.end()};

	auto sum_abs_if_candidate = [&branch_candidates](auto const& row_vals, auto const& row_cols) {
		auto const n_cols = row_cols.size();
		value_type sum = 0.;
		for (std::size_t i = 0; i < n_cols; ++i) {
			auto const var = SCIPcolGetVar(row_cols[i]);
			if (branch_candidates.find(var) != branch_candidates.end()) {
				sum += std::abs(row_vals[i]);
			}
		}
		return sum;
	};

	auto safe_inv = [](auto const x) { return x != 0. ? 1. / x : 1.; };

	xt::xtensor<value_type, 2> weights{{lp_rows.size(), 4}, std::nan("")};
	auto weights_iter = weights.begin();

	for (auto const row : lp_rows) {
		if (row_is_active(scip, row)) {
			auto const row_vals = scip_row_get_vals(row);
			*(weights_iter++) = 1.;
			*(weights_iter++) = safe_inv(sum_abs(row_vals));
			*(weights_iter++) = safe_inv(sum_abs_if_candidate(row_vals, scip_row_get_cols(row)));
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
 * We consider 4 weighting schemes for an active constraint: unit weight, inverse of the sum of the
 * coefficients of all variables in constraint, inverse of the sum of the coefficients of only
 * candidate variables in constraint, dual cost of the constraint.
 * Given the absolute value of the coefficients of xj in the active constraints, we compute the sum,
 * mean, stdev., max. and min. of those values, for each of the weighting schemes.
 * We also compute the weighted number of active constraints that xj is in, with the same 4
 * weightings.
 */
auto stats_for_active_constraint_coefficients(
	Scip* const scip,
	nonstd::span<scip::Row*> const rows,
	nonstd::span<scip::real> const coefficients,
	xt::xtensor<value_type, 2> active_rows_weights) noexcept -> Features<24> {

	std::array<StatsFeatures, 4> weights_stats{};
	for (auto& stats : weights_stats) {
		stats.min = std::numeric_limits<decltype(stats.min)>::max();
		stats.max = std::numeric_limits<decltype(stats.max)>::min();
	}

	auto const n_rows = rows.size();
	std::size_t n_active_rows = 0UL;
	for (std::size_t row_idx = 0; row_idx < n_rows; ++row_idx) {
		auto const row = rows[row_idx];
		auto const row_lp_idx = SCIProwGetLPPos(row);
		auto const abs_coef = std::abs(coefficients[row_idx]);

		if (row_is_active(scip, row)) {
			n_active_rows++;

			for (std::size_t weight_idx = 0; weight_idx < weights_stats.size(); ++weight_idx) {
				auto const weight = active_rows_weights(row_lp_idx, weight_idx);
				assert(!std::isnan(weight));  // If NaN likely hit a maked value
				auto const weighted_abs_coef = weight * abs_coef;

				auto& stats = weights_stats[weight_idx];
				stats.count += weight;
				stats.sum += weighted_abs_coef;
				stats.min = std::min(stats.min, weighted_abs_coef);
				stats.max = std::max(stats.max, weighted_abs_coef);
			}
		}
	}

	if (n_active_rows == 0UL) {
		return {0.};
	}

	for (auto& stats : weights_stats) {
		stats.mean = stats.sum / static_cast<value_type>(n_active_rows);
	}

	for (std::size_t row_idx = 0; row_idx < n_rows; ++row_idx) {
		auto const row = rows[row_idx];
		auto const row_lp_idx = SCIProwGetLPPos(row);
		auto const abs_coef = std::abs(coefficients[row_idx]);
		if (row_is_active(scip, row)) {
			for (std::size_t weight_idx = 0; weight_idx < weights_stats.size(); ++weight_idx) {
				auto const weight = active_rows_weights(row_lp_idx, weight_idx);
				assert(!std::isnan(weight));  // If NaN likely hit a maked value
				auto const weighted_abs_coef = weight * abs_coef;

				auto& stats = weights_stats[weight_idx];
				stats.variance = square(weighted_abs_coef - stats.mean);
			}
		}
	}

	return {
		weights_stats[0].count,    weights_stats[0].sum, weights_stats[0].mean,
		weights_stats[0].variance, weights_stats[0].min, weights_stats[0].max,
		weights_stats[1].count,    weights_stats[1].sum, weights_stats[1].mean,
		weights_stats[1].variance, weights_stats[1].min, weights_stats[1].max,
		weights_stats[2].count,    weights_stats[2].sum, weights_stats[2].mean,
		weights_stats[2].variance, weights_stats[2].min, weights_stats[2].max,
		weights_stats[3].count,    weights_stats[3].sum, weights_stats[3].mean,
		weights_stats[3].variance, weights_stats[3].min, weights_stats[3].max,
	};
}

/******************************
 *  Main extraction function  *
 ******************************/

auto extract_feat(scip::Model const& model) {
	std::size_t constexpr n_features = 72UL;
	std::size_t const n_candidate_vars = model.pseudo_branch_cands().size();

	xt::xtensor<value_type, 2> observation{{n_candidate_vars, n_features}, std::nan("")};

	auto const scip = model.get_scip_ptr();
	auto const active_rows_weights = stats_for_active_constraint_coefficients_weights(model);

	auto iter = observation.begin();
	for (auto const var : model.pseudo_branch_cands()) {
		auto* const col = SCIPvarGetCol(var);
		auto const rows = scip_col_get_rows(col);
		auto const coefficients = scip_col_get_vals(col);
		auto const root_stats = static_stats_for_constraint_degree_stats(rows);
		// Static features
		iter = copy(objective_function_coefficient(col), iter);
		iter = copy(number_constraints(col), iter);
		iter = copy(static_stats_for_constraint_degree(root_stats), iter);
		iter = copy(stats_for_constraint_positive_coefficients(coefficients), iter);
		iter = copy(stats_for_constraint_negative_coefficients(coefficients), iter);
		// Dynamic features
		iter = copy(slack_ceil_distances(scip, col), iter);
		iter = copy(pseudocosts(scip, var), iter);
		iter = copy(infeasibility_statistics(var), iter);
		iter = copy(dynamic_stats_for_constraint_degree(rows, root_stats), iter);
		iter = copy(min_max_for_ratios_constraint_coeffs_rhs(scip, rows, coefficients), iter);
		iter = copy(min_max_for_one_to_all_coefficient_ratios(rows, coefficients), iter);
		iter = copy(
			stats_for_active_constraint_coefficients(scip, rows, coefficients, active_rows_weights),
			iter);
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(static_cast<std::size_t>(iter - observation.begin()) == observation.size());

	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto Khalil2016::obtain_observation(scip::Model& model) -> nonstd::optional<Khalil2016Obs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		return extract_feat(model);
	}
	return {};
}

}  // namespace observation
}  // namespace ecole
