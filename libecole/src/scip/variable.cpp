#include <cstddef>

#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

VarProxy::VarProxy(SCIP_Var* value) noexcept : Proxy(value) {}

double VarProxy::ub() const noexcept {
	return SCIPvarGetUbLocal(value);
}

VarProxy const VarProxy::None = VarProxy(nullptr);

}  // namespace scip
}  // namespace ecole
