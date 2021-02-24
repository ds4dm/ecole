#pragma once

#include <nonstd/span.hpp>
#include <scip/scip.h>

namespace ecole::scip {

auto get_rows(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_ROW*>;
auto get_vals(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_Real>;

}  // namespace ecole::scip
