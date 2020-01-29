#include <scip/scip.h>

#include "ecole/scip/column.hpp"

namespace ecole {
namespace scip {

real ColProxy::ub() const noexcept {
	return SCIPcolGetUb(value);
}

real ColProxy::lb() const noexcept {
	return SCIPcolGetLb(value);
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
