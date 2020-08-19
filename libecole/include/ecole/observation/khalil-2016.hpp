#pragma once

#include <cstddef>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

using Khalil2016Obs = xt::xtensor<double, 2>;

class Khalil2016 : public ObservationFunction<nonstd::optional<Khalil2016Obs>> {
public:
	struct Feature {
		static constexpr std::size_t n_static = 18;
		static constexpr std::size_t n_dynamic = 54;
		static constexpr std::size_t n_features = n_static + n_dynamic;

		enum struct Static : std::size_t {
			/** Objective function coeffs. (3) */
			obj_coef = 0,
			obj_coef_pos_part,
			obj_coef_neg_part,
			/** Num. constraints (1) */
			n_rows,
			/** Stats. for constraint degrees (4) */
			rows_deg_mean,
			rows_deg_var,
			rows_deg_min,
			rows_deg_max,
			/** Stats. for constraint coeffs. (10) */
			rows_pos_coefs_count,
			rows_pos_coefs_mean,
			rows_pos_coefs_var,
			rows_pos_coefs_min,
			rows_pos_coefs_max,
			rows_neg_coefs_count,
			rows_neg_coefs_mean,
			rows_neg_coefs_var,
			rows_neg_coefs_min,
			rows_neg_coefs_max,
		};

		enum struct Dynamic : std::size_t {
			/** Slack and ceil distances (2) */
			slack = n_static,  // Index start after static features in observation
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
			n_branching_up,
			n_branching_down,
			/** Stats. for constraint degrees (7) */
			rows_deg_mean,
			rows_deg_var,
			rows_deg_min,
			rows_deg_max,
			rows_deg_mean_ratio,
			rows_deg_min_ratio,
			rows_deg_max_ratio,
			/** Min/max for ratios of constraint coeffs. to RHS (4) */
			pos_coef_rhs_ratio_max,
			pos_coef_rhs_ratio_min,
			neg_coef_rhs_ratio_max,
			neg_coef_rhs_ratio_min,
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
			active_coef_weight1_var,
			active_coef_weight1_min,
			active_coef_weight1_max,
			active_coef_weight2_count,
			active_coef_weight2_sum,
			active_coef_weight2_mean,
			active_coef_weight2_var,
			active_coef_weight2_min,
			active_coef_weight2_max,
			active_coef_weight3_count,
			active_coef_weight3_sum,
			active_coef_weight3_mean,
			active_coef_weight3_var,
			active_coef_weight3_min,
			active_coef_weight3_max,
			active_coef_weight4_count,
			active_coef_weight4_sum,
			active_coef_weight4_mean,
			active_coef_weight4_var,
			active_coef_weight4_min,
			active_coef_weight4_max,
		};
	};

	void reset(scip::Model& model) override;
	nonstd::optional<Khalil2016Obs> obtain_observation(scip::Model& model) override;

private:
	xt::xtensor<Khalil2016Obs::value_type, 2> static_features;
};

}  // namespace observation
}  // namespace ecole
