#include <type_traits>

#include <cmath>
#include <cstddef>
#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xadapt.hpp>
#include <xtensor/xnorm.hpp>
#include <xtensor/xview.hpp>

#include "ecole/exception.hpp"
#include "ecole/observation/milpbipartite.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using xmatrix = decltype(MilpBipartiteObs::variable_features);
using value_type = xmatrix::value_type;
using coo_xmatrix = utility::coo_matrix<value_type>;

using VariableFeatures = MilpBipartiteObs::VariableFeatures;
using ConstraintFeatures = MilpBipartiteObs::ConstraintFeatures;

/******************************************
 *  Variable extraction functions         *
 ******************************************/

/* Computes the L2 norm of the objective.
	 This is done by hand because SCIPgetObjNorm is not available for all stages (need >= SCIP_STAGE_TRANSFORMED) */
SCIP_Real obj_l2_norm(SCIP* const scip, scip::Model& model) noexcept {
	SCIP_Real norm = 0.;

	if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
		norm = SCIPgetObjNorm(scip);
	} else {
		// If too early, this must be done by hand
		for (auto* variable : model.variables()) {
			norm += std::pow(SCIPvarGetObj(variable), 2);
		}
		norm = std::sqrt(norm);
	}

	return norm > 0 ? norm : 1.;
}

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Features>
void set_static_features_for_var(
	Features&& out,
	SCIP* const scip,
	SCIP_VAR* const var,
	std::optional<value_type> obj_norm = {}) {
	double const objsense = (SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE) ? 1. : -1.;

	out[idx(VariableFeatures::objective)] = objsense * SCIPvarGetObj(var);
	if (obj_norm.has_value()) {
		out[idx(VariableFeatures::objective)] /= obj_norm.value();
	}
	// One-hot enconding of variable type
	out[idx(VariableFeatures::is_type_binary)] = 0.;
	out[idx(VariableFeatures::is_type_integer)] = 0.;
	out[idx(VariableFeatures::is_type_implicit_integer)] = 0.;
	out[idx(VariableFeatures::is_type_continuous)] = 0.;
	switch (SCIPvarGetType(var)) {
	case SCIP_VARTYPE_BINARY:
		out[idx(VariableFeatures::is_type_binary)] = 1.;
		break;
	case SCIP_VARTYPE_INTEGER:
		out[idx(VariableFeatures::is_type_integer)] = 1.;
		break;
	case SCIP_VARTYPE_IMPLINT:
		out[idx(VariableFeatures::is_type_implicit_integer)] = 1.;
		break;
	case SCIP_VARTYPE_CONTINUOUS:
		out[idx(VariableFeatures::is_type_continuous)] = 1.;
		break;
	default:
		assert(false);  // All enum cases must be handled
	}

	auto const lower_bound = SCIPvarGetLbLocal(var);
	if (SCIPisInfinity(scip, std::abs(lower_bound))) {
		out[idx(VariableFeatures::has_lower_bound)] = 0.;
		out[idx(VariableFeatures::lower_bound)] = 0.;
	} else {
		out[idx(VariableFeatures::has_lower_bound)] = 1.;
		out[idx(VariableFeatures::lower_bound)] = lower_bound;
	}

	auto const upper_bound = SCIPvarGetUbLocal(var);
	if (SCIPisInfinity(scip, std::abs(upper_bound))) {
		out[idx(VariableFeatures::has_upper_bound)] = 0.;
		out[idx(VariableFeatures::upper_bound)] = 0.;
	} else {
		out[idx(VariableFeatures::has_upper_bound)] = 1.;
		out[idx(VariableFeatures::upper_bound)] = upper_bound;
	}
}

void set_features_for_all_vars(xmatrix& out, scip::Model& model, bool normalize) {
	auto* const scip = model.get_scip_ptr();

	// Contant reused in every iterations
	auto const obj_norm = normalize ? std::optional{obj_l2_norm(scip, model)} : std::nullopt;

	auto const variables = model.variables();
	auto const n_vars = variables.size();
	for (std::size_t var_idx = 0; var_idx < n_vars; ++var_idx) {
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(var_idx));
		set_static_features_for_var(features, scip, variables[var_idx], obj_norm);
	}
}

/** Convert a xtensor of size (N) into an xtensor of size (N, 1) without copy. */
template <typename T> auto vec_to_col(xt::xtensor<T, 1>&& t) -> xt::xtensor<T, 2> {
	return xt::xtensor<T, 2>{std::move(t.storage()), {t.size(), 1}, {1, 0}};
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto MilpBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<MilpBipartiteObs> {
	if (model.stage() < SCIP_STAGE_SOLVING) {
		auto [edge_features, constraint_features] = scip::get_all_constraints(model.get_scip_ptr(), normalize);

		auto variable_features = xmatrix::from_shape({model.variables().size(), MilpBipartiteObs::n_variable_features});
		set_features_for_all_vars(variable_features, model, normalize);

		return MilpBipartiteObs{
			std::move(variable_features),
			vec_to_col(std::move(constraint_features)),
			std::move(edge_features),
		};
	}
	return {};
}

}  // namespace ecole::observation
