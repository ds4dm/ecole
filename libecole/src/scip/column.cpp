#include <scip/scip.h>

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

VarProxy ColProxy::var() const noexcept {
	return VarProxy(scip, SCIPcolGetVar(value));
}

}  // namespace scip
}  // namespace ecole
