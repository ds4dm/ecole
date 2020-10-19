#pragma once

#include <memory>
#include <optional>

#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/scip_cons.h>

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

class ConsReleaser {
public:
	ConsReleaser(SCIP* scip_) noexcept : scip(scip_){};
	void operator()(SCIP_CONS* ptr);

private:
	SCIP* scip = nullptr;
};

/**
 * Create a linear constraint with automatic management (RAII).
 *
 * The constraint is returned in a unique_ptr that will automatically call SCIPreleaseCons on deletion.
 * Even if the release is done automatically the SCIP semantics are not changed and the constraint must not outlive the
 * SCIP pointer (it is needed to release the constraint).
 *
 * The arguments are forwarded to SCIPcreateConsBasicLinear.
 */
auto create_cons_basic_linear(
	SCIP* scip,
	char const* name,
	std::size_t n_vars,
	SCIP_VAR const* const* vars,
	SCIP_Real const* vals,
	SCIP_Real lhs,
	SCIP_Real rhs) -> std::unique_ptr<SCIP_CONS, ConsReleaser>;

auto cons_get_rhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real>;

auto cons_get_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real>;

}  // namespace ecole::scip
