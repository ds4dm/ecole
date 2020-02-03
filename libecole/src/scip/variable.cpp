#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy const VarProxy::None = VarProxy(nullptr, nullptr);

optional<real> VarProxy::ub_local() const noexcept {
	auto const ub_val = SCIPvarGetUbLocal(value);
	if (SCIPisInfinity(scip, REALABS(ub_val)))
		return {};
	else
		return ub_val;
}

optional<real> VarProxy::lb_local() const noexcept {
	auto const lb_val = SCIPvarGetLbLocal(value);
	if (SCIPisInfinity(scip, REALABS(lb_val)))
		return {};
	else
		return lb_val;
}

optional<real> VarProxy::best_sol_val() const noexcept {
	auto const sol = SCIPgetBestSol(scip);
	if (sol != nullptr)
		return SCIPgetSolVal(scip, sol, value);
	else
		return {};
}

optional<real> VarProxy::avg_sol() const noexcept {
	if (SCIPgetBestSol(scip) == nullptr)
		return SCIPvarGetAvgSol(value);
	else
		return {};
}

var_type VarProxy::type_() const noexcept {
	return SCIPvarGetType(value);
}

}  // namespace scip
}  // namespace ecole
