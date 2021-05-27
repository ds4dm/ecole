#include <algorithm>
#include <map>
#include <optional>
#include <utility>

#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>
#include <scip/scip.h>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/hutter-2011.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"

#include "utility/math.hpp"

namespace ecole::observation {

namespace {

namespace views = ranges::views;

using Features = Hutter2011Obs::Features;
using value_type = decltype(Hutter2011Obs::features)::value_type;

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Tensor> void set_problem_size(Tensor&& out, scip::Model const& model) {
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	out[idx(Features::nb_variables)] = static_cast<value_type>(SCIPgetNVars(scip));
	out[idx(Features::nb_constraints)] = static_cast<value_type>(SCIPgetNConss(scip));
	// SCIPgetNNZs return 0 at in problem stage so we use the degree statistics in `set_variable_degrees` instead.
}

template <typename Tensor> void set_variable_degrees(Tensor&& out, scip::Model const& model) {
	// FIXME use a cache friendly robin hood has map such as
	// https://github.com/Tessil/robin-map
	// Fill a map variable -> degree with all variables and 0.
	auto var_degrees = std::map<SCIP_VAR const*, std::size_t>{};
	auto const variables = model.variables();
	std::for_each(variables.begin(), variables.end(), [&var_degrees](auto var) { var_degrees[var] = 0; });

	// Compute degrees by iterating over the constraints.
	for (auto* const cons : model.constraints()) {
		auto maybe_cons_vars = scip::get_cons_vars(model.get_scip_ptr(), cons);
		if (maybe_cons_vars.has_value()) {
			auto const cons_vars = std::move(maybe_cons_vars).value();
			std::for_each(cons_vars.begin(), cons_vars.end(), [&var_degrees](auto var) { var_degrees[var]++; });
		}
	}

	auto const stats = utility::compute_stats(var_degrees | views::values);
	out[idx(Features::variable_node_degree_mean)] = stats.mean;
	out[idx(Features::variable_node_degree_max)] = stats.max;
	out[idx(Features::variable_node_degree_min)] = stats.min;
	out[idx(Features::variable_node_degree_std)] = stats.stddev;
	// See `set_problem_size`.
	out[idx(Features::nb_nonzero_coefs)] = stats.count;
}

template <typename Tensor> void set_constraint_degrees(Tensor&& out, scip::Model const& model) {
	auto cons_degree = [&model](auto cons) {
		return static_cast<value_type>(scip::get_cons_n_vars(model.get_scip_ptr(), cons).value_or(0));
	};
	auto const constraints = model.constraints();
	auto const stats = utility::compute_stats(constraints | views::transform(cons_degree));
	out[idx(Features::constraint_node_degree_mean)] = stats.mean;
	out[idx(Features::constraint_node_degree_max)] = stats.max;
	out[idx(Features::constraint_node_degree_min)] = stats.min;
	out[idx(Features::constraint_node_degree_std)] = stats.stddev;
}

auto extract_features(scip::Model& model) {
	xt::xtensor<value_type, 1> observation({Hutter2011Obs::n_features});
	set_problem_size(observation, model);
	set_variable_degrees(observation, model);
	set_constraint_degrees(observation, model);
	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

// void Hutter2011::before_reset(scip::Model& /* model */) {
// 	static_features = decltype(static_features){};
// }

auto Hutter2011::extract(scip::Model& model, bool /* done */) -> std::optional<Hutter2011Obs> {
	if (model.get_stage() >= SCIP_STAGE_SOLVING) {
		return {};
	}
	return {{extract_features(model)}};
}

}  // namespace ecole::observation
