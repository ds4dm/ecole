#pragma once

#include <optional>

#include <nonstd/span.hpp>
#include <scip/scip.h>

namespace ecole::scip {

auto get_unshifted_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;
auto get_unshifted_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;

auto is_at_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;
auto is_at_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;

auto get_cols(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_COL*>;
auto get_vals(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_Real>;

}  // namespace ecole::scip
