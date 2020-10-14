#pragma once

#include <memory>

#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/scip_cons.h>

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

/**
 * Create a linear constraint with automatic management (RAII).
 *
 * The constraint is returned in a unique_ptr that will automatically call SCIPreleaseCons on deletion.
 * Even if the release is done automatically the SCIP semantics are not changed and the constraint must not outlive the
 * SCIP pointer (it is needed to release the constraint).
 *
 * The arguments are forwarded to SCIPcreateConsBasicLinear.
 */
template <typename... Args> auto create_cons_basic_linear(SCIP* scip, Args&&... args) {
	SCIP_CONS* cons = nullptr;
	scip::call(SCIPcreateConsBasicLinear, scip, &cons, std::forward<Args>(args)...);
	return std::unique_ptr{cons, [scip](SCIP_CONS* ptr) { scip::call(SCIPreleaseCons, scip, &ptr); }};
}

}  // namespace ecole::scip
