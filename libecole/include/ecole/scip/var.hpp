#pragma once

#include <memory>

#include <scip/scip.h>
#include <scip/scip_var.h>

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

/**
 * Create a variable with automatic management (RAII).
 *
 * The variable is returned in a unique_ptr that will automatically call SCIPreleaseVar on deletion.
 * Even if the release is done automatically the SCIP semantics are not changed and the variable must not outlive the
 * SCIP pointer (it is needed to release the variable).
 *
 * The arguments are forwarded to SCIPcreateVarBasic.
 */
template <typename... Args> auto create_var_basic(SCIP* scip, Args&&... args) {
	SCIP_VAR* var = nullptr;
	scip::call(SCIPcreateVarBasic, scip, &var, std::forward<Args>(args)...);
	auto deleter = [scip](SCIP_VAR* ptr) { scip::call(SCIPreleaseVar, scip, &ptr); };
	return std::unique_ptr<SCIP_VAR, decltype(deleter)>{var, deleter};
}

}  // namespace ecole::scip
