#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy const VarProxy::None = VarProxy(nullptr, nullptr);

real VarProxy::ub() const noexcept {
	return SCIPvarGetUbLocal(value);
}

real VarProxy::lb() const noexcept {
	return SCIPvarGetLbLocal(value);
}

}  // namespace scip
}  // namespace ecole
