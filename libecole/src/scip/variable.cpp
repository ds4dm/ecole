#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy const VarProxy::None = VarProxy(nullptr, nullptr);

real VarProxy::ub_local() const noexcept {
	return SCIPvarGetUbLocal(value);
}

real VarProxy::lb_local() const noexcept {
	return SCIPvarGetLbLocal(value);
}

var_type VarProxy::type_() const noexcept {
	return SCIPvarGetType(value);
}

}  // namespace scip
}  // namespace ecole
