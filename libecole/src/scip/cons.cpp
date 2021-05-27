#include <stdexcept>

#include "ecole/scip/cons.hpp"

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
	return rhs;
}

auto cons_get_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	SCIP_Bool success = FALSE;
	auto const lhs = SCIPconsGetLhs(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &success);
	if (success == FALSE) {
		return {};
	}
	return lhs;
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

auto get_cons_vars(SCIP const* scip, SCIP_CONS const* cons, nonstd::span<SCIP_VAR const*> out) -> bool {
	auto const maybe_n_vars = get_cons_n_vars(scip, cons);
	if (!maybe_n_vars.has_value()) {
		return false;
	}
	auto const n_vars = maybe_n_vars.value();
	if (out.size() < n_vars) {
		throw std::invalid_argument{"Out memory is not large enough to fit variables."};
	}
	SCIP_Bool success = false;
	scip::call(
		SCIPgetConsVars,
		const_cast<SCIP*>(scip),
		const_cast<SCIP_CONS*>(cons),
		const_cast<SCIP_VAR**>(out.data()),
		static_cast<int>(out.size()),
		&success);
	return success;
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
	SCIP_Bool success = false;
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

}  // namespace ecole::scip
