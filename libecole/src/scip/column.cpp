#include <scip/scip.h>
#include <scip/struct_lp.h>

#include "ecole/scip/column.hpp"

namespace ecole {
namespace scip {

optional<real> ColProxy::ub() const noexcept {
	auto const ub_val = SCIPcolGetUb(value);
	if (SCIPisInfinity(scip, REALABS(ub_val)))
		return {};
	else
		return ub_val;
}

optional<real> ColProxy::lb() const noexcept {
	auto const lb_val = SCIPcolGetLb(value);
	if (SCIPisInfinity(scip, REALABS(lb_val)))
		return {};
	else
		return lb_val;
}

real ColProxy::reduced_cost() const noexcept {
	return SCIPgetColRedcost(scip, value);
}

real ColProxy::obj() const noexcept {
	return SCIPcolGetObj(value);
}

real ColProxy::prim_sol() const noexcept {
	return SCIPcolGetPrimsol(value);
}

real ColProxy::prim_sol_frac() const noexcept {
	return SCIPfeasFrac(scip, prim_sol());
}

bool ColProxy::is_prim_sol_at_lb() const noexcept {
	auto const lb_val = lb();
	if (lb_val)
		return SCIPisEQ(scip, prim_sol(), lb_val.value());
	else
		return false;
}

bool ColProxy::is_prim_sol_at_ub() const noexcept {
	auto const ub_val = ub();
	if (ub_val)
		return SCIPisEQ(scip, prim_sol(), ub_val.value());
	else
		return false;
}

base_stat ColProxy::basis_status() const noexcept {
	return SCIPcolGetBasisStatus(value);
}

VarProxy ColProxy::var() const noexcept {
	return VarProxy(scip, SCIPcolGetVar(value));
}

int ColProxy::age() const noexcept {
	return value->age;
}

}  // namespace scip
}  // namespace ecole
