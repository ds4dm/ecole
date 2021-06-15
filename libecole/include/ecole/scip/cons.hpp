#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <xtensor/xtensor.hpp>

#include <nonstd/span.hpp>
#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/scip_cons.h>

#include "ecole/scip/utils.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::scip {

/** Scip deleter for Cons pointers. */
class ConsReleaser {
public:
	/** Capture the SCIP pointer but does not extend its lifetime. */
	ConsReleaser(SCIP* scip_) noexcept : scip(scip_){};

	/** Call SCIPconsRelease */
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
auto cons_get_finite_rhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real>;
auto cons_get_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real>;
auto cons_get_finite_lhs(SCIP const* scip, SCIP_CONS const* cons) noexcept -> std::optional<SCIP_Real>;

auto get_cons_n_vars(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::size_t>;
auto get_cons_vars(SCIP* scip, SCIP_CONS* cons, nonstd::span<SCIP_VAR*> out) -> bool;
auto get_cons_vars(SCIP const* scip, SCIP_CONS const* cons, nonstd::span<SCIP_VAR const*> out) -> bool;
auto get_cons_vars(SCIP* scip, SCIP_CONS* cons) -> std::optional<std::vector<SCIP_VAR*>>;
auto get_cons_vars(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::vector<SCIP_VAR const*>>;
auto get_cons_vals(SCIP const* scip, SCIP_CONS const* cons, nonstd::span<SCIP_Real> out) -> bool;
auto get_cons_vals(SCIP const* scip, SCIP_CONS const* cons) -> std::optional<std::vector<SCIP_Real>>;

auto get_vals_linear(SCIP const* scip, SCIP_CONS const* cons) noexcept -> nonstd::span<SCIP_Real const>;
auto get_vars_linear(SCIP const* scip, SCIP_CONS const* cons) noexcept -> nonstd::span<SCIP_VAR const* const>;

auto get_constraint_linear_coefs(SCIP* scip, SCIP_CONS* constraint) -> std::optional<
	std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>>>;
auto get_constraint_coefs(SCIP* scip, SCIP_CONS* constraint)
	-> std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>>;
auto get_all_constraints(SCIP* scip, bool normalize = false, bool include_variable_bounds = false)
	-> std::tuple<utility::coo_matrix<SCIP_Real>, xt::xtensor<SCIP_Real, 1>>;

}  // namespace ecole::scip
