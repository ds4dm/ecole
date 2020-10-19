#pragma once

#include <memory>

#include <scip/scip.h>
#include <scip/scip_var.h>

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

/** Scip deleter for Var pointers. */
class VarReleaser {
public:
	/** Capture the SCIP pointer but does not extend its lifetime. */
	VarReleaser(SCIP* scip_) noexcept : scip(scip_){};

	/** Call SCIPvarRelease */
	void operator()(SCIP_VAR* ptr);

private:
	SCIP* scip = nullptr;
};

/**
 * Create a variable with automatic management (RAII).
 *
 * The variable is returned in a unique_ptr that will automatically call SCIPreleaseVar on deletion.
 * Even if the release is done automatically the SCIP semantics are not changed and the variable must not outlive the
 * SCIP pointer (it is needed to release the variable).
 *
 * The arguments are forwarded to SCIPcreateVarBasic.
 */
auto create_var_basic(SCIP* scip, char const* name, SCIP_Real lb, SCIP_Real ub, SCIP_Real obj, SCIP_VARTYPE vartype)
	-> std::unique_ptr<SCIP_VAR, VarReleaser>;

}  // namespace ecole::scip
