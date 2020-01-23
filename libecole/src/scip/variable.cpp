#include <cstddef>

#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy::VarProxy(SCIP_Var* value) noexcept : Proxy(value) {}

auto VarProxy::ub() const noexcept -> decltype(SCIPvarGetUbLocal(nullptr)) {
	return SCIPvarGetUbLocal(value);
}

auto VarProxy::lb() const noexcept -> decltype(SCIPvarGetLbLocal(nullptr)) {
	return SCIPvarGetLbLocal(value);
}

VarProxy const VarProxy::None = VarProxy(nullptr);

}  // namespace scip
}  // namespace ecole
