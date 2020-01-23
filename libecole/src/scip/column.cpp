#include <scip/scip.h>

#include "ecole/scip/column.hpp"

namespace ecole {
namespace scip {

auto ColProxy::ub() const noexcept -> decltype(SCIPcolGetUb(nullptr)) {
	return SCIPcolGetUb(value);
}

auto ColProxy::lb() const noexcept -> decltype(SCIPcolGetLb(nullptr)) {
	return SCIPcolGetLb(value);
}

auto ColProxy::reduced_cost() const noexcept
	-> decltype(SCIPgetColRedcost(nullptr, nullptr)) {
	return SCIPgetColRedcost(scip, value);
}

VarProxy ColProxy::var() const noexcept {
	return VarProxy(scip, SCIPcolGetVar(value));
}

}  // namespace scip
}  // namespace ecole
