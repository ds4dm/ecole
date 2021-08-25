#pragma once

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"

namespace ecole::scip {

ECOLE_EXPORT auto get_rows(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_ROW*>;
ECOLE_EXPORT auto get_vals(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_Real>;

}  // namespace ecole::scip
