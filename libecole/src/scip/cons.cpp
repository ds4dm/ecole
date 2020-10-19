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
	return {cons, ConsReleaser(scip)};
}

auto cons_get_rhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	SCIP_Bool success = FALSE;
	auto const rhs = SCIPconsGetRhs(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &success);
	if (success == FALSE) {
		return {};
	}
	return rhs;
}

auto cons_get_lhs(SCIP* const scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real> {
	SCIP_Bool success = FALSE;
	auto const lhs = SCIPconsGetLhs(const_cast<SCIP*>(scip), const_cast<SCIP_CONS*>(cons), &success);
	if (success == FALSE) {
		return {};
	}
	return lhs;
}

}  // namespace ecole::scip
