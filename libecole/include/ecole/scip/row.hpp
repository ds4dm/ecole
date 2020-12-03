#pragma once

#include <optional>

#include <scip/scip.h>

namespace ecole::scip {

auto get_unshifted_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;
auto get_unshifted_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;

auto is_at_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;
auto is_at_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;

}  // namespace ecole::scip
