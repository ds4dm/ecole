#include <cstddef>

#include <scip/cons_linear.h>
#include <scip/scip.h>

#include "unit-tests.hpp"

namespace ecole::instance {

namespace {

auto same_lhs(SCIP* scip1, SCIP_Cons* constraint1, SCIP* scip2, SCIP_Cons* constraint2) noexcept -> bool {
	SCIP_Bool success1 = FALSE;
	SCIP_Bool success2 = FALSE;
	auto const lhs1 = SCIPconsGetLhs(scip1, constraint1, &success1);
	auto const lhs2 = SCIPconsGetLhs(scip2, constraint2, &success2);
	if (success1 == FALSE) {
		return false;
	}
	if (success2 == FALSE) {
		return false;
	}
	return lhs1 == lhs2;
}

auto same_rhs(SCIP* scip1, SCIP_Cons* constraint1, SCIP* scip2, SCIP_Cons* constraint2) noexcept -> bool {
	SCIP_Bool success1 = FALSE;
	SCIP_Bool success2 = FALSE;
	auto const rhs1 = SCIPconsGetRhs(scip1, constraint1, &success1);
	auto const rhs2 = SCIPconsGetRhs(scip2, constraint2, &success2);
	if (success1 == FALSE) {
		return false;
	}
	if (success2 == FALSE) {
		return false;
	}
	return rhs1 == rhs2;
}

auto same_constraint_permutation(SCIP* scip1, SCIP_Cons* constraint1, SCIP* scip2, SCIP_Cons* constraint2) noexcept
	-> bool {
	if (!same_lhs(scip1, constraint1, scip2, constraint2)) {
		return false;
	}
	if (!same_rhs(scip1, constraint1, scip2, constraint2)) {
		return false;
	}

	auto* const cons_values1 = SCIPgetValsLinear(scip1, constraint1);
	auto const cons_n_values1 = static_cast<std::size_t>(SCIPgetNVarsLinear(scip1, constraint1));
	auto* const cons_values2 = SCIPgetValsLinear(scip2, constraint2);
	auto const cons_n_values2 = static_cast<std::size_t>(SCIPgetNVarsLinear(scip2, constraint2));

	if (cons_n_values1 != cons_n_values2) {
		return false;
	}

	for (std::size_t i = 0; i < cons_n_values1; ++i) {
		if (cons_values1[i] != cons_values2[i]) {
			return false;
		}
	}

	return true;
}

}  // namespace

auto same_problem_permutation(scip::Model const& model1, scip::Model const& model2) noexcept -> bool {
	auto* scip1 = const_cast<SCIP*>(model1.get_scip_ptr());
	auto* scip2 = const_cast<SCIP*>(model2.get_scip_ptr());

	if (SCIPgetObjsense(scip1) != SCIPgetObjsense(scip2)) {
		return false;
	}

	if (model1.variables().size() != model2.variables().size()) {
		return false;
	}

	auto const constraints1 = model1.constraints();
	auto const constraints2 = model2.constraints();
	if (constraints1.size() != constraints2.size()) {
		return false;
	}
	for (std::size_t i = 0; i < constraints1.size(); ++i) {
		if (!same_constraint_permutation(scip1, constraints1[i], scip2, constraints2[i])) {
			return false;
		}
	}

	return true;
}

}  // namespace ecole::instance
