#include <iostream>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/khalil-2016.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Khalil2016 unit tests", "[unit][obs]") {
	observation::unit_tests(observation::Khalil2016{});
}

template <typename Tensor, typename T = typename Tensor::value_type>
auto in_interval(Tensor const& tensor, T const& lower, T const& upper) {
	// Must take bounds by reference because they are captured by reference in the xexpression
	return (lower <= tensor) && (tensor <= upper);
}

TEST_CASE("Khalil2016 return correct observation", "[obs]") {
	using Feature = observation::Khalil2016::Feature;
	using Static = observation::Khalil2016::Feature::Static;
	using Dynamic = observation::Khalil2016::Feature::Dynamic;

	auto obs_func = observation::Khalil2016{};
	auto model = get_solving_model();
	obs_func.reset(model);
	auto const optional_obs = obs_func.obtain_observation(model);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features has correct shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.shape(0) == model.pseudo_branch_cands().size());
		REQUIRE(obs.shape(1) == Feature::n_features);
	}

	SECTION("No features are NaN or infinite") {
		auto const& obs = optional_obs.value();
		REQUIRE_FALSE(xt::any(xt::isnan(obs)));
		REQUIRE_FALSE(xt::any(xt::isinf(obs)));
	}

	SECTION("Observation has correct values") {
		auto const& obs = optional_obs.value();
		auto col = [&obs](auto feature) { return xt::col(obs, static_cast<std::ptrdiff_t>(feature)); };

		SECTION("Objective function coefficients") {
			REQUIRE(xt::all(col(Static::obj_coef_pos_part) >= 0));
			REQUIRE(xt::all(col(Static::obj_coef_neg_part) >= 0));
			auto const& pos_minus_neg = col(Static::obj_coef_pos_part) - col(Static::obj_coef_pos_part);
			REQUIRE(xt::all(xt::equal(col(Static::obj_coef), pos_minus_neg)));
		}

		SECTION("Number of constraint") { REQUIRE(xt::all(col(Static::n_rows) >= 0)); }

		SECTION("Static stats for constraint degree") {
			REQUIRE(xt::all(col(Static::rows_deg_mean) >= 0));
			REQUIRE(xt::all(col(Static::rows_deg_stddev) >= 0));
			REQUIRE(xt::all(col(Static::rows_deg_min) >= 0));
			REQUIRE(xt::all(col(Static::rows_deg_max) >= 0));
			REQUIRE(xt::all(col(Static::rows_deg_mean) >= col(Static::rows_deg_min)));
			REQUIRE(xt::all(col(Static::rows_deg_mean) <= col(Static::rows_deg_max)));
		}

		SECTION("Stats for constraint positive coefficients") {
			REQUIRE(xt::all(col(Static::rows_pos_coefs_count) >= 0));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_mean) >= 0));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_stddev) >= 0));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_min) >= 0));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_max) >= 0));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_mean) >= col(Static::rows_pos_coefs_min)));
			REQUIRE(xt::all(col(Static::rows_pos_coefs_mean) <= col(Static::rows_pos_coefs_max)));
		}

		SECTION("Stats for constraint negative coefficients") {
			REQUIRE(xt::all(col(Static::rows_neg_coefs_count) >= 0));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_mean) <= 0));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_stddev) <= 0));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_min) <= 0));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_max) <= 0));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_mean) >= col(Static::rows_neg_coefs_min)));
			REQUIRE(xt::all(col(Static::rows_neg_coefs_mean) <= col(Static::rows_neg_coefs_max)));
		}

		SECTION("Slack and ceil distance") {
			REQUIRE(xt::all(in_interval(col(Dynamic::slack), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::ceil_dist), 0, 1)));
		}

		SECTION("Pseudocosts") {
			REQUIRE(xt::all(col(Dynamic::pseudocost_ratio) >= 0));
			auto const pseudo_sum = col(Dynamic::pseudocost_down) + col(Dynamic::pseudocost_up);
			REQUIRE(xt::all(xt::equal(pseudo_sum, col(Dynamic::pseudocost_sum))));
		}

		SECTION("Infeasibility statistics") {
			REQUIRE(xt::all(col(Dynamic::n_cutoff_up) >= 0));
			REQUIRE(xt::all(col(Dynamic::n_cutoff_down) >= 0));
			REQUIRE(xt::all(in_interval(col(Dynamic::n_cutoff_up_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::n_cutoff_down_ratio), 0, 1)));
		}

		SECTION("Dynamic stats for constraint degree") {
			REQUIRE(xt::all(col(Dynamic::rows_deg_mean) >= 0));
			REQUIRE(xt::all(col(Dynamic::rows_deg_stddev) >= 0));
			REQUIRE(xt::all(col(Dynamic::rows_deg_min) >= 0));
			REQUIRE(xt::all(col(Dynamic::rows_deg_max) >= 0));
			REQUIRE(xt::all(col(Dynamic::rows_deg_mean) >= col(Dynamic::rows_deg_min)));
			REQUIRE(xt::all(col(Dynamic::rows_deg_mean) <= col(Dynamic::rows_deg_max)));
			REQUIRE(xt::all(in_interval(col(Dynamic::rows_deg_mean_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::rows_deg_min_ratio), 0, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::rows_deg_max_ratio), 0, 1)));
		}

		SECTION("Min/max for ratios of constraint coeffs. to RHS") {
			REQUIRE(xt::all(in_interval(col(Dynamic::coef_pos_rhs_ratio_min), -1, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::coef_pos_rhs_ratio_max), -1, 1)));
			REQUIRE(xt::all(col(Dynamic::coef_pos_rhs_ratio_min) <= col(Dynamic::coef_pos_rhs_ratio_max)));
			REQUIRE(xt::all(in_interval(col(Dynamic::coef_neg_rhs_ratio_min), -1, 1)));
			REQUIRE(xt::all(in_interval(col(Dynamic::coef_neg_rhs_ratio_max), -1, 1)));
			REQUIRE(xt::all(col(Dynamic::coef_neg_rhs_ratio_min) <= col(Dynamic::coef_neg_rhs_ratio_max)));
		}

		SECTION("Stats. for active constraint coefficients") {
			REQUIRE(xt::all(col(Dynamic::active_coef_weight1_count) >= 0));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight1_mean) >= col(Dynamic::active_coef_weight1_min)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight1_mean) <= col(Dynamic::active_coef_weight1_max)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight2_count) >= 0));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight2_mean) >= col(Dynamic::active_coef_weight2_min)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight2_mean) <= col(Dynamic::active_coef_weight2_max)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight3_count) >= 0));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight3_mean) >= col(Dynamic::active_coef_weight3_min)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight3_mean) <= col(Dynamic::active_coef_weight3_max)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight4_count) >= 0));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight4_mean) >= col(Dynamic::active_coef_weight4_min)));
			REQUIRE(xt::all(col(Dynamic::active_coef_weight4_mean) <= col(Dynamic::active_coef_weight4_max)));
		}
	}
}
