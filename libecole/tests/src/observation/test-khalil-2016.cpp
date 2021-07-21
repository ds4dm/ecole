#include <catch2/catch.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>
#include <scip/scip.h>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/khalil-2016.hpp"
#include "ecole/tweak/range.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

namespace views = ranges::views;

using namespace ecole;

TEST_CASE("Khalil2016 unit tests", "[unit][obs]") {
	observation::unit_tests(observation::Khalil2016{});
}

template <typename Tensor, typename T = typename Tensor::value_type>
auto in_interval(Tensor const& tensor, T const& lower, T const& upper) {
	// Must take bounds by reference because they are captured by reference in the xexpression
	return (lower <= tensor) && (tensor <= upper);
}

/** Get the features of the pseudo candidate only. */
template <typename Tensor, typename Range>
auto obs_pseudo_cands(Tensor const& obs_features, Range const& pseudo_cands_idx) -> Tensor {
	auto filtered_features = Tensor::from_shape({pseudo_cands_idx.size(), obs_features.shape()[1]});
	for (auto const [idx, var_idx] : views::enumerate(pseudo_cands_idx)) {
		xt::row(filtered_features, static_cast<std::ptrdiff_t>(idx)) = xt::row(obs_features, var_idx);
	}
	return filtered_features;
}

TEST_CASE("Khalil2016 return correct observation", "[obs]") {
	using Features = observation::Khalil2016Obs::Features;

	auto obs_func = observation::Khalil2016{};
	auto model = get_model();
	obs_func.before_reset(model);
	advance_to_stage(model, SCIP_STAGE_SOLVING);
	auto const optional_obs = obs_func.extract(model, false);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features has correct shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.features.shape(0) == model.variables().size());
		REQUIRE(obs.features.shape(1) == observation::Khalil2016Obs::n_features);
	}

	SECTION("Observation has correct values") {
		auto const& obs = optional_obs.value();
		auto obs_pseudo =
			obs_pseudo_cands(obs.features, views::transform(model.pseudo_branch_cands(), SCIPvarGetProbindex));
		auto col = [&obs_pseudo](auto feat) { return xt::col(obs_pseudo, static_cast<std::ptrdiff_t>(feat)); };

		SECTION("No pseudo_candidate features are NaN or infinite") {
			for (auto* var : model.pseudo_branch_cands()) {
				auto const var_idx = SCIPvarGetProbindex(var);
				REQUIRE_FALSE(xt::any(xt::isnan(xt::row(obs.features, var_idx))));
				REQUIRE_FALSE(xt::any(xt::isinf(xt::row(obs.features, var_idx))));
			}
		}

		SECTION("Objective function coefficients") {
			REQUIRE(xt::all(col(Features::obj_coef_pos_part) >= 0));
			REQUIRE(xt::all(col(Features::obj_coef_neg_part) >= 0));
			auto const& pos_minus_neg = col(Features::obj_coef_pos_part) - col(Features::obj_coef_pos_part);
			REQUIRE(xt::all(xt::equal(col(Features::obj_coef), pos_minus_neg)));
		}

		SECTION("Number of constraint") { REQUIRE(xt::all(col(Features::n_rows) >= 0)); }

		SECTION("Static stats for constraint degree") {
			REQUIRE(xt::all(col(Features::rows_deg_mean) >= 0));
			REQUIRE(xt::all(col(Features::rows_deg_stddev) >= 0));
			REQUIRE(xt::all(col(Features::rows_deg_min) >= 0));
			REQUIRE(xt::all(col(Features::rows_deg_max) >= 0));
			REQUIRE(xt::all(col(Features::rows_deg_mean) >= col(Features::rows_deg_min)));
			REQUIRE(xt::all(col(Features::rows_deg_mean) <= col(Features::rows_deg_max)));
		}

		SECTION("Stats for constraint positive coefficients") {
			REQUIRE(xt::all(col(Features::rows_pos_coefs_count) >= 0));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_mean) >= 0));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_stddev) >= 0));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_min) >= 0));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_max) >= 0));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_mean) >= col(Features::rows_pos_coefs_min)));
			REQUIRE(xt::all(col(Features::rows_pos_coefs_mean) <= col(Features::rows_pos_coefs_max)));
		}

		SECTION("Stats for constraint negative coefficients") {
			REQUIRE(xt::all(col(Features::rows_neg_coefs_count) >= 0));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_mean) <= 0));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_stddev) <= 0));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_min) <= 0));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_max) <= 0));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_mean) >= col(Features::rows_neg_coefs_min)));
			REQUIRE(xt::all(col(Features::rows_neg_coefs_mean) <= col(Features::rows_neg_coefs_max)));
		}

		SECTION("Slack and ceil distance") {
			REQUIRE(xt::all(in_interval(col(Features::slack), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Features::ceil_dist), 0, 1)));
		}

		SECTION("Pseudocosts") {
			REQUIRE(xt::all(col(Features::pseudocost_ratio) >= 0));
			auto const pseudo_sum = col(Features::pseudocost_down) + col(Features::pseudocost_up);
			REQUIRE(xt::all(xt::equal(pseudo_sum, col(Features::pseudocost_sum))));
		}

		SECTION("Infeasibility statistics") {
			REQUIRE(xt::all(col(Features::n_cutoff_up) >= 0));
			REQUIRE(xt::all(col(Features::n_cutoff_down) >= 0));
			REQUIRE(xt::all(in_interval(col(Features::n_cutoff_up_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Features::n_cutoff_down_ratio), 0, 1)));
		}

		SECTION("Dynamic stats for constraint degree") {
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_mean) >= 0));
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_stddev) >= 0));
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_min) >= 0));
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_max) >= 0));
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_mean) >= col(Features::rows_dynamic_deg_min)));
			REQUIRE(xt::all(col(Features::rows_dynamic_deg_mean) <= col(Features::rows_dynamic_deg_max)));
			REQUIRE(xt::all(in_interval(col(Features::rows_dynamic_deg_mean_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Features::rows_dynamic_deg_min_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Features::rows_dynamic_deg_max_ratio), 0, 1)));
		}

		SECTION("Min/max for ratios of constraint coeffs. to RHS") {
			REQUIRE(xt::all(in_interval(col(Features::coef_pos_rhs_ratio_min), -1, 1)));
			REQUIRE(xt::all(in_interval(col(Features::coef_pos_rhs_ratio_max), -1, 1)));
			REQUIRE(xt::all(col(Features::coef_pos_rhs_ratio_min) <= col(Features::coef_pos_rhs_ratio_max)));
			REQUIRE(xt::all(in_interval(col(Features::coef_neg_rhs_ratio_min), -1, 1)));
			REQUIRE(xt::all(in_interval(col(Features::coef_neg_rhs_ratio_max), -1, 1)));
			REQUIRE(xt::all(col(Features::coef_neg_rhs_ratio_min) <= col(Features::coef_neg_rhs_ratio_max)));
		}

		SECTION("Stats. for active constraint coefficients") {
			REQUIRE(xt::all(col(Features::active_coef_weight1_count) >= 0));
			REQUIRE(xt::all(col(Features::active_coef_weight1_mean) >= col(Features::active_coef_weight1_min)));
			REQUIRE(xt::all(col(Features::active_coef_weight1_mean) <= col(Features::active_coef_weight1_max)));
			REQUIRE(xt::all(col(Features::active_coef_weight2_count) >= 0));
			REQUIRE(xt::all(col(Features::active_coef_weight2_mean) >= col(Features::active_coef_weight2_min)));
			REQUIRE(xt::all(col(Features::active_coef_weight2_mean) <= col(Features::active_coef_weight2_max)));
			REQUIRE(xt::all(col(Features::active_coef_weight3_count) >= 0));
			REQUIRE(xt::all(col(Features::active_coef_weight3_mean) >= col(Features::active_coef_weight3_min)));
			REQUIRE(xt::all(col(Features::active_coef_weight3_mean) <= col(Features::active_coef_weight3_max)));
			REQUIRE(xt::all(col(Features::active_coef_weight4_count) >= 0));
			REQUIRE(xt::all(col(Features::active_coef_weight4_mean) >= col(Features::active_coef_weight4_min)));
			REQUIRE(xt::all(col(Features::active_coef_weight4_mean) <= col(Features::active_coef_weight4_max)));
		}
	}
}
