#include <cstddef>

#include <scip/scip.h>

#include "ecole/scip/variable.hpp"

namespace ecole {
namespace scip {

double VarProxy::ub() const noexcept {
	return SCIPvarGetUbOriginal(const_cast<SCIP_Var*>(value));
}
} // namespace scip
} // namespace ecole
