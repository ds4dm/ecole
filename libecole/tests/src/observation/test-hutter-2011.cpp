#include <algorithm>
#include <cmath>
#include <type_traits>

#include <catch2/catch.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/hutter-2011.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

template <typename T> auto is_integer(T val) -> bool {
	if constexpr (std::is_integral_v<T>) {
		return true;
	} else {
		return std::floor(val) == val;
	}
}

template <typename T> auto is_positive_integer(T val) -> bool {
	return is_integer(val) && (val >= 0);
}

template <typename... T> auto is_sorted(T... vals) -> bool {
	auto arr = std::array<std::common_type_t<T...>, sizeof...(vals)>{vals...};
	return std::is_sorted(arr.begin(), arr.end());
}

TEST_CASE("Hutter2011 unit tests", "[unit][obs]") {
	observation::unit_tests(observation::Hutter2011{});
}

TEST_CASE("Hutter2011 return correct observation", "[obs]") {
	using Features = observation::Hutter2011Obs::Features;

	auto obs_func = observation::Hutter2011{};
	auto model = get_model();
	obs_func.before_reset(model);
	auto const optional_obs = obs_func.extract(model, false);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features has correct shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.features.shape(0) == observation::Hutter2011Obs::n_features);
	}

	SECTION("No features are NaN or infinite") {
		auto const& obs = optional_obs.value();
		REQUIRE_FALSE(xt::any(xt::isnan(obs.features)));
		REQUIRE_FALSE(xt::any(xt::isinf(obs.features)));
	}

	SECTION("Observation has correct values") {
		auto const& obs = optional_obs.value();
		auto get_feature = [&obs](auto feat) { return obs.features[static_cast<std::size_t>(feat)]; };

		SECTION("Problem size features") {
			REQUIRE(is_positive_integer(get_feature(Features::nb_variables)));
			REQUIRE(is_positive_integer(get_feature(Features::nb_constraints)));
			REQUIRE(is_positive_integer(get_feature(Features::nb_nonzero_coefs)));
		}

		SECTION("Variable-constraint graph features") {
			{  // Variables
				REQUIRE(0. <= get_feature(Features::variable_node_degree_std));
				auto const min_degree = get_feature(Features::variable_node_degree_min);
				auto const max_degree = get_feature(Features::variable_node_degree_max);
				auto const mean_degree = get_feature(Features::variable_node_degree_mean);
				auto const nb_cons = get_feature(Features::nb_constraints);
				REQUIRE(is_integer(min_degree));
				REQUIRE(is_integer(max_degree));
				REQUIRE(is_sorted(0., min_degree, mean_degree, max_degree, nb_cons));
			}
			{  // Constraints
				REQUIRE(0. <= get_feature(Features::constraint_node_degree_std));
				auto const min_degree = get_feature(Features::constraint_node_degree_min);
				auto const max_degree = get_feature(Features::constraint_node_degree_max);
				auto const mean_degree = get_feature(Features::constraint_node_degree_mean);
				auto const nb_var = get_feature(Features::nb_variables);
				REQUIRE(is_integer(min_degree));
				REQUIRE(is_integer(max_degree));
				REQUIRE(is_sorted(0., min_degree, mean_degree, max_degree, nb_var));
			}
		}

		SECTION("Variable graph features") {
			REQUIRE(0. <= get_feature(Features::node_degree_std));
			auto const min_degree = get_feature(Features::node_degree_min);
			auto const max_degree = get_feature(Features::node_degree_max);
			auto const mean_degree = get_feature(Features::node_degree_mean);
			auto const nb_var = get_feature(Features::nb_variables);
			REQUIRE(is_integer(min_degree));
			REQUIRE(is_integer(max_degree));
			REQUIRE(is_sorted(0., min_degree, mean_degree, max_degree, nb_var));
			auto const q25_degree = get_feature(Features::node_degree_25q);
			auto const q75_degree = get_feature(Features::node_degree_75q);
			REQUIRE(is_sorted(min_degree, q25_degree, q75_degree, max_degree));
			REQUIRE(is_sorted(0., get_feature(Features::edge_density), 1.));
		}

		SECTION("LP based features") {
			REQUIRE(get_feature(Features::lp_slack_mean) <= get_feature(Features::lp_slack_max));
			REQUIRE(0. <= get_feature(Features::lp_slack_l2));
		}

		SECTION("Objective function features") {
			REQUIRE(0. <= get_feature(Features::objective_coef_m_std));
			REQUIRE(0. <= get_feature(Features::objective_coef_n_std));
			REQUIRE(0. <= get_feature(Features::objective_coef_sqrtn_std));
		}

		SECTION("Linear constraint matrix features") {
			REQUIRE(0. <= get_feature(Features::constraint_coef_std));
			REQUIRE(0. <= get_feature(Features::constraint_var_coef_mean));
			REQUIRE(0. <= get_feature(Features::constraint_var_coef_std));
		}

		SECTION("Variable type features") {
			REQUIRE(0. <= get_feature(Features::discrete_vars_support_size_mean));
			REQUIRE(0. <= get_feature(Features::discrete_vars_support_size_std));
			REQUIRE(is_sorted(0., get_feature(Features::ratio_unbounded_discrete_vars), 1.));
			REQUIRE(is_sorted(0., get_feature(Features::ratio_continuous_vars), 1.));
		}
	}
}
