#include <cmath>

#include <scip/scip.h>
#include <scip/struct_lp.h>

#include "ecole/scip/row.hpp"

namespace ecole {
namespace scip {

real RowProxy::constant() const noexcept {
	return SCIProwGetConstant(value);
}

optional<real> RowProxy::lhs() const noexcept {
	auto const lhs_val = SCIProwGetLhs(value);
	if (SCIPisInfinity(scip, REALABS(lhs_val)))
		return {};
	else
		return lhs_val - constant();
}

optional<real> RowProxy::rhs() const noexcept {
	auto const rhs_val = SCIProwGetRhs(value);
	if (SCIPisInfinity(scip, REALABS(rhs_val)))
		return {};
	else
		return rhs_val - constant();
}

int RowProxy::n_lp_nonz() const noexcept {
	return SCIProwGetNLPNonz(value);
}

real RowProxy::l2_norm() const noexcept {
	return SCIProwGetNorm(value);
}

real RowProxy::obj_cos_sim() const noexcept {
	auto const norm_prod = SCIProwGetNorm(value) * SCIPgetObjNorm(scip);
	if (SCIPisPositive(scip, norm_prod))
		return value->objprod / norm_prod;
	else
		return 0.;
}

bool RowProxy::is_local() const noexcept {
	return SCIProwIsLocal(value);
}

bool RowProxy::is_modifiable() const noexcept {
	return SCIProwIsModifiable(value);
}

bool RowProxy::is_removable() const noexcept {
	return SCIProwIsRemovable(value);
}

real RowProxy::dual_sol() const noexcept {
	return SCIProwGetDualsol(value);
}

base_stat RowProxy::basis_status() const noexcept {
	return SCIProwGetBasisStatus(value);
}

int RowProxy::age() const noexcept {
	return SCIProwGetAge(value);
}

real RowProxy::lp_activity() const noexcept {
	return SCIPgetRowLPActivity(scip, value) - constant();
}

bool RowProxy::is_at_lhs() const noexcept {
	auto const activity = SCIPgetRowLPActivity(scip, value);
	auto const lhs_val = SCIProwGetLhs(value);
	return SCIPisEQ(scip, activity, lhs_val);
}

bool RowProxy::is_at_rhs() const noexcept {
	auto const activity = SCIPgetRowLPActivity(scip, value);
	auto const rhs_val = SCIProwGetRhs(value);
	return SCIPisEQ(scip, activity, rhs_val);
}

}  // namespace scip
}  // namespace ecole
