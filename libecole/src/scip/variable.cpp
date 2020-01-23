#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy const VarProxy::None = VarProxy(nullptr, nullptr);

auto VarProxy::ub() const noexcept -> decltype(SCIPvarGetUbLocal(nullptr)) {
	return SCIPvarGetUbLocal(value);
}

auto VarProxy::lb() const noexcept -> decltype(SCIPvarGetLbLocal(nullptr)) {
	return SCIPvarGetLbLocal(value);
}


}  // namespace scip
}  // namespace ecole
