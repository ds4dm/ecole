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
inline auto create_cons_basic_linear(
	SCIP* scip,
	char const* name,
	std::size_t n_vars,
	SCIP_VAR const* const* vars,
	SCIP_Real const* vals,
	SCIP_Real lhs,
	SCIP_Real rhs) {

	SCIP_CONS* cons = nullptr;
	scip::call(
		SCIPcreateConsBasicLinear,
		scip,
		&cons,
		name,
		static_cast<int>(n_vars),
		const_cast<SCIP_VAR**>(vars),
		const_cast<SCIP_Real*>(vals),
		lhs,
		rhs);
	auto deleter = [scip](SCIP_CONS* ptr) { scip::call(SCIPreleaseCons, scip, &ptr); };
	return std::unique_ptr<SCIP_CONS, decltype(deleter)>{cons, deleter};
}

}  // namespace ecole::scip
