#include <cmath>

#include "ecole/scip/row.hpp"

namespace ecole::scip {

namespace {

template <typename T> auto not_const(T const* ptr) noexcept -> T* {
	return const_cast<T*>(ptr);
}

}  // namespace

auto get_unshifted_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real> {
	auto const rhs_val = SCIProwGetRhs(not_const(row));
	if (SCIPisInfinity(not_const(scip), std::abs(rhs_val))) {
		return {};
	}
	return rhs_val - SCIProwGetConstant(not_const(row));
}

auto get_unshifted_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> std::optional<SCIP_Real> {
	auto const lhs_val = SCIProwGetLhs(not_const(row));
	if (SCIPisInfinity(not_const(scip), std::abs(lhs_val))) {
		return {};
	}
	return lhs_val - SCIProwGetConstant(not_const(row));
}

auto is_at_rhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool {
	auto const activity = SCIPgetRowLPActivity(not_const(scip), not_const(row));
	auto const rhs_val = SCIProwGetRhs(not_const(row));
	return SCIPisEQ(not_const(scip), activity, rhs_val);
}

auto is_at_lhs(SCIP const* scip, SCIP_ROW const* row) noexcept -> bool {
	auto const activity = SCIPgetRowLPActivity(not_const(scip), not_const(row));
	auto const lhs_val = SCIProwGetLhs(not_const(row));
	return SCIPisEQ(not_const(scip), activity, lhs_val);
}

auto get_cols(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_COL*> {
	auto const n_cols = SCIProwGetNNonz(not_const(row));
	return {SCIProwGetCols(not_const(row)), static_cast<std::size_t>(n_cols)};
}

auto get_vals(SCIP_ROW const* row) noexcept -> nonstd::span<SCIP_Real> {
	auto const n_cols = SCIProwGetNNonz(not_const(row));
	return {SCIProwGetVals(not_const(row)), static_cast<std::size_t>(n_cols)};
}

}  // namespace ecole::scip
