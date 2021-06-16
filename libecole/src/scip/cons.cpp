#include <cmath>
#include <fmt/format.h>
#include <stdexcept>
#include <xtensor/xadapt.hpp>
#include <xtensor/xnorm.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/scip/cons.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::scip {

void ConsReleaser::operator()(SCIP_CONS* ptr) {
	scip::call(SCIPreleaseCons, scip, &ptr);
}

auto create_cons_basic_linear(
	SCIP* scip,
	char const* name,
	std::size_t n_vars,
	SCIP_VAR const* const* vars,
	SCIP_Real const* vals,
	SCIP_Real lhs,
	SCIP_Real rhs) -> std::unique_ptr<SCIP_CONS, ConsReleaser> {

	SCIP_CONS* cons = nullptr;
	scip::call(
		SCIPcreateConsBasicLinear,
		scip,
		&cons,
		name,
		static_cast<int>(n_vars),
		const_cast<SCIP_VAR**>(vars),
		const_cast<SCIP_Real*>(vals),
		lhs,
		rhs);
	return {cons, ConsReleaser{scip}};
}

auto cons_get_rhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	SCIP_Bool success = FALSE;
	auto const rhs = SCIPconsGetRhs(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &success);
	if (success == FALSE) {
		return {};
	}
	return {rhs};
}

auto cons_get_finite_rhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	if (auto rhs = cons_get_rhs(scip, cons); rhs.has_value()) {
		if (!SCIPisInfinity(const_cast<SCIP*>(scip), std::abs(rhs.value()))) {
			return rhs;
		}
	}
	return {};
}

auto cons_get_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	SCIP_Bool success = FALSE;
	auto const lhs = SCIPconsGetLhs(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &success);
	if (success == FALSE) {
		return {};
	}
	return {lhs};
}

auto cons_get_finite_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	if (auto lhs = cons_get_lhs(scip, cons); lhs.has_value()) {
		if (!SCIPisInfinity(const_cast<SCIP*>(scip), std::abs(lhs.value()))) {
			return lhs;
		}
	}
	return {};
}

auto get_cons_n_vars(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::size_t> {
	SCIP_Bool success = false;
	int n_vars = 0;
	scip::call(SCIPgetConsNVars, const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &n_vars, &success);
	if (!success) {
		return {};
	}
	assert(n_vars >= 0);
	return {static_cast<std::size_t>(n_vars)};
}

auto get_cons_vars(SCIP* scip, SCIP_CONS* cons, nonstd::span<SCIP_VAR*> out) -> bool {
	auto const maybe_n_vars = get_cons_n_vars(scip, cons);
	if (!maybe_n_vars.has_value()) {
		return false;
	}
	auto const n_vars = maybe_n_vars.value();
	if (out.size() < n_vars) {
		throw std::invalid_argument{"Out memory is not large enough to fit variables."};
	}
	SCIP_Bool success = FALSE;
	scip::call(SCIPgetConsVars, scip, cons, out.data(), static_cast<int>(out.size()), &success);
	return success;
}

auto get_cons_vars(SCIP const* scip, SCIP_CONS const* cons, nonstd::span<SCIP_VAR const*> out) -> bool {
	return get_cons_vars(
		const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), {const_cast<SCIP_VAR**>(out.data()), out.size()});
}

auto get_cons_vars(SCIP* scip, SCIP_CONS* cons) -> std::optional<std::vector<SCIP_VAR*>> {
	if (auto const n_vars = get_cons_n_vars(scip, cons); n_vars.has_value()) {
		auto vars = std::vector<SCIP_VAR*>(n_vars.value());
		if (get_cons_vars(scip, cons, vars)) {
			return {std::move(vars)};
		}
	}
	return {};
}

auto get_cons_vars(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::vector<SCIP_VAR const*>> {
	if (auto const n_vars = get_cons_n_vars(scip, cons); n_vars.has_value()) {
		auto vars = std::vector<SCIP_VAR const*>(n_vars.value());
		if (get_cons_vars(scip, cons, vars)) {
			return {std::move(vars)};
		}
	}
	return {};
}

auto get_cons_vals(SCIP const* scip, SCIP_CONS const* cons, nonstd::span<SCIP_Real> out) -> bool {
	auto const maybe_n_vars = get_cons_n_vars(scip, cons);
	if (!maybe_n_vars.has_value()) {
		return false;
	}
	auto const n_vars = maybe_n_vars.value();
	if (out.size() < n_vars) {
		throw std::invalid_argument{"Out memory is not large enough to fit variables."};
	}
	SCIP_Bool success = FALSE;
	scip::call(
		SCIPgetConsVals,
		const_cast<SCIP*>(scip),
		const_cast<SCIP_CONS*>(cons),
		out.data(),
		static_cast<int>(out.size()),
		&success);
	return success;
}

auto get_cons_vals(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::vector<SCIP_Real>> {
	if (auto const n_vars = get_cons_n_vars(scip, cons); n_vars.has_value()) {
		auto vals = std::vector<SCIP_Real>(n_vars.value());
		if (get_cons_vals(scip, cons, vals)) {
			return {std::move(vals)};
		}
	}
	return {};
}

auto get_vals_linear(SCIP const* scip, SCIP_CONS const* cons) noexcept -> nonstd::span<SCIP_Real const> {
	return {
		SCIPgetValsLinear(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons)),
		static_cast<std::size_t>(SCIPgetNVarsLinear(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons))),
	};
}

auto get_vars_linear(SCIP const* scip, SCIP_CONS const* cons) noexcept -> nonstd::span<SCIP_VAR const* const> {
	return {
		SCIPgetVarsLinear(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons)),
		static_cast<std::size_t>(SCIPgetNVarsLinear(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons))),
	};
}

/**
 * Obtains the variables involved in a linear constraint and their coefficients in the constraint
 */
auto get_constraint_linear_coefs(SCIP* const scip, SCIP_CONS* const constraint) -> std::optional<
	std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>>> {
	SCIP_Bool success = false;
	int n_constraint_variables;
	int n_active_variables;
	SCIP_Real constant_offset = 0;
	int requiredsize = 0;

	// Find how many active variables and constraint variables there are (for allocation)
	scip::call(SCIPgetConsNVars, scip, constraint, &n_constraint_variables, &success);
	if (!success) {
		return std::nullopt;
	}
	n_active_variables = SCIPgetNVars(scip);

	// Allocate buffers large enough to hold future variables and coefficients
	auto const buffer_size = static_cast<std::size_t>(std::max(n_constraint_variables, n_active_variables));
	auto variables = std::vector<SCIP_VAR*>(buffer_size);
	auto coefficients = std::vector<SCIP_Real>(buffer_size);

	// Get the variables and their coefficients in the constraint
	scip::call(SCIPgetConsVars, scip, constraint, variables.data(), static_cast<int>(buffer_size), &success);
	if (!success) {
		return std::nullopt;
	}
	scip::call(SCIPgetConsVals, scip, constraint, coefficients.data(), static_cast<int>(buffer_size), &success);
	if (!success) {
		return std::nullopt;
	}

	// If we are in SCIP_STAGE_TRANSFORMED or later, the variables in the constraint might be inactive
	// Re-express the coefficients in terms of active variables
	if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
		scip::call(
			SCIPgetProbvarLinearSum,
			scip,
			variables.data(),
			coefficients.data(),
			&n_constraint_variables,
			static_cast<int>(buffer_size),
			&constant_offset,
			&requiredsize,
			true);
	}

	variables.resize(static_cast<std::size_t>(n_constraint_variables));
	coefficients.resize(static_cast<std::size_t>(n_constraint_variables));

	// Obtain the left and right hand side if their are finite and shift them accordingly.
	auto lhs = scip::cons_get_finite_lhs(scip, constraint);
	if (lhs.has_value()) {
		lhs = lhs.value() - constant_offset;
	}

	auto rhs = scip::cons_get_finite_rhs(scip, constraint);
	if (rhs.has_value()) {
		rhs = rhs.value() - constant_offset;
	}

	return {{variables, coefficients, lhs, rhs}};
}

auto get_constraint_coefs(SCIP* const scip, SCIP_CONS* const constraint)
	-> std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>> {
	auto constraint_data = get_constraint_linear_coefs(scip, constraint);
	if (constraint_data.has_value()) {  // Constraint must be linear
		return constraint_data.value();
	}
	throw Exception(fmt::format(
		"Constraint {} cannot be expressed as a single linear constraint (type \"{}\"), MilpBipartite observation "
		"cannot be extracted.",
		SCIPconsGetPos(constraint),
		SCIPconshdlrGetName(SCIPconsGetHdlr(constraint))));
}

SCIP_Real cons_l2_norm(std::vector<SCIP_Real> const& constraint_coefs) {
	auto xt_constraint_coefs = xt::adapt(constraint_coefs, {constraint_coefs.size()});

	auto const norm = xt::norm_l2(xt_constraint_coefs)();
	return norm > 0. ? norm : 1.;
}

auto get_all_constraints(SCIP* const scip, bool normalize, bool include_variable_bounds)
	-> std::tuple<utility::coo_matrix<SCIP_Real>, xt::xtensor<SCIP_Real, 1>> {
	auto* const variables = SCIPgetVars(scip);
	auto* const constraints = SCIPgetConss(scip);
	auto nb_variables = static_cast<std::size_t>(SCIPgetNVars(scip));
	auto nb_constraints = static_cast<std::size_t>(SCIPgetNConss(scip));

	std::size_t n_rows = 0;

	std::vector<SCIP_Real> values;
	std::vector<std::size_t> column_indices;
	std::vector<std::size_t> row_indices;
	std::vector<SCIP_Real> biases;

	// For each constraint
	for (std::size_t cons_idx = 0; cons_idx < nb_constraints; ++cons_idx) {
		auto* const constraint = constraints[cons_idx];
		auto [constraint_vars, constraint_coefs, lhs, rhs] = get_constraint_coefs(scip, constraint);
		SCIP_Real const constraint_norm = normalize ? cons_l2_norm(constraint_coefs) : 1.;

		// Inequality has a left hand side?
		if (lhs.has_value()) {
			for (std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
				SCIP_Real value = constraint_coefs[cons_var_idx];
				int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);

				values.push_back(-value);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
			}
			if (normalize) {
				biases.push_back(-lhs.value() / constraint_norm);
			} else {
				biases.push_back(-lhs.value());
			}
			n_rows++;
		}
		// Inequality has a right hand side?
		if (rhs.has_value()) {
			for (std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
				SCIP_Real value = constraint_coefs[cons_var_idx];
				int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);

				values.push_back(value);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
			}
			if (normalize) {
				biases.push_back(rhs.value() / constraint_norm);
			} else {
				biases.push_back(rhs.value());
			}
			n_rows++;
		}
	}

	if (include_variable_bounds) {
		// Add variable bounds as additional constraints
		for (std::size_t var_idx = 0; var_idx < nb_variables; ++var_idx) {
			auto lb = SCIPvarGetLbGlobal(variables[var_idx]);
			auto ub = SCIPvarGetUbGlobal(variables[var_idx]);
			if (!SCIPisInfinity(scip, std::abs(lb))) {
				values.push_back(-1.);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
				biases.push_back(-lb);
				n_rows++;
			}
			if (!SCIPisInfinity(scip, std::abs(ub))) {
				values.push_back(1.);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
				biases.push_back(ub);
				n_rows++;
			}
		}
	}

	// Turn values and indices into xt::xarray's
	auto const nnz = values.size();

	utility::coo_matrix<SCIP_Real> constraint_matrix{};
	constraint_matrix.values = xt::adapt(std::move(values), {nnz});
	constraint_matrix.indices = decltype(utility::coo_matrix<SCIP_Real>::indices)::from_shape({2, nnz});
	xt::row(constraint_matrix.indices, 0) = xt::adapt(std::move(row_indices), {nnz});
	xt::row(constraint_matrix.indices, 1) = xt::adapt(std::move(column_indices), {nnz});
	constraint_matrix.shape = {n_rows, nb_variables};

	xt::xtensor<SCIP_Real, 1> constraint_biases = xt::adapt(std::move(biases), {n_rows});

	return std::tuple{std::move(constraint_matrix), std::move(constraint_biases)};
}

}  // namespace ecole::scip
