#pragma once

#include <cstddef>
#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

struct Khalil2016Obs {
	static inline std::size_t constexpr n_static_features = 18;
	static inline std::size_t constexpr n_dynamic_features = 54;
	static inline std::size_t constexpr n_features = n_static_features + n_dynamic_features;

	enum struct Features : std::size_t {
		/** Static features */
		/** Objective function coeffs. (3) */
		obj_coef = 0,
		obj_coef_pos_part,
		obj_coef_neg_part,
		/** Num. constraints (1) */
		n_rows,
		/** Stats. for constraint degrees (4) */
		rows_deg_mean,
		rows_deg_stddev,
		rows_deg_min,
		rows_deg_max,
		/** Stats. for constraint coeffs. (10) */
		rows_pos_coefs_count,
		rows_pos_coefs_mean,
		rows_pos_coefs_stddev,
		rows_pos_coefs_min,
		rows_pos_coefs_max,
		rows_neg_coefs_count,
		rows_neg_coefs_mean,
		rows_neg_coefs_stddev,
		rows_neg_coefs_min,
		rows_neg_coefs_max,

		/** Dynamic features */
		/** Slack and ceil distances (2) */
		slack,
		ceil_dist,
		/** Pseudocosts (5) */
		pseudocost_up,
		pseudocost_down,
		pseudocost_ratio,
		pseudocost_sum,
		pseudocost_product,
		/** Infeasibility statistics (4) */
		n_cutoff_up,
		n_cutoff_down,
		n_cutoff_up_ratio,
		n_cutoff_down_ratio,
		/** Stats. for constraint degrees (7) */
		rows_dynamic_deg_mean,
		rows_dynamic_deg_stddev,
		rows_dynamic_deg_min,
		rows_dynamic_deg_max,
		rows_dynamic_deg_mean_ratio,
		rows_dynamic_deg_min_ratio,
		rows_dynamic_deg_max_ratio,
		/** Min/max for ratios of constraint coeffs. to RHS (4) */
		coef_pos_rhs_ratio_min,
		coef_pos_rhs_ratio_max,
		coef_neg_rhs_ratio_min,
		coef_neg_rhs_ratio_max,
		/** Min/max for one-to-all coefficient ratios (8) */
		pos_coef_pos_coef_ratio_min,
		pos_coef_pos_coef_ratio_max,
		pos_coef_neg_coef_ratio_min,
		pos_coef_neg_coef_ratio_max,
		neg_coef_pos_coef_ratio_min,
		neg_coef_pos_coef_ratio_max,
		neg_coef_neg_coef_ratio_min,
		neg_coef_neg_coef_ratio_max,
		/** Stats. for active constraint coefficients (24) */
		active_coef_weight1_count,
		active_coef_weight1_sum,
		active_coef_weight1_mean,
		active_coef_weight1_stddev,
		active_coef_weight1_min,
		active_coef_weight1_max,
		active_coef_weight2_count,
		active_coef_weight2_sum,
		active_coef_weight2_mean,
		active_coef_weight2_stddev,
		active_coef_weight2_min,
		active_coef_weight2_max,
		active_coef_weight3_count,
		active_coef_weight3_sum,
		active_coef_weight3_mean,
		active_coef_weight3_stddev,
		active_coef_weight3_min,
		active_coef_weight3_max,
		active_coef_weight4_count,
		active_coef_weight4_sum,
		active_coef_weight4_mean,
		active_coef_weight4_stddev,
		active_coef_weight4_min,
		active_coef_weight4_max,
	};

	xt::xtensor<double, 2> features;
};

class Khalil2016 : public ObservationFunction<std::optional<Khalil2016Obs>> {
public:
	void before_reset(scip::Model& model) override;

	std::optional<Khalil2016Obs> extract(scip::Model& model, bool done) override;

private:
	xt::xtensor<double, 2> static_features;
};

}  // namespace ecole::observation
