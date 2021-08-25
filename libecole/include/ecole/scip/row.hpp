#pragma once

#include <optional>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"

namespace ecole::scip {

ECOLE_EXPORT auto get_unshifted_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;
ECOLE_EXPORT auto get_unshifted_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real>;

ECOLE_EXPORT auto is_at_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;
ECOLE_EXPORT auto is_at_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool;

ECOLE_EXPORT auto get_cols(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_COL*>;
ECOLE_EXPORT auto get_vals(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_Real>;

}  // namespace ecole::scip
