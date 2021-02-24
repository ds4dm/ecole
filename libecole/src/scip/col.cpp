#include "ecole/scip/col.hpp"

namespace ecole::scip {

namespace {

template <typename T> auto not_const(T const* ptr) noexcept -> T* {
	return const_cast<T*>(ptr);
}

}  // namespace

auto get_rows(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_ROW*> {
	auto const n_rows = SCIPcolGetNNonz(not_const(col));
	return {SCIPcolGetRows(not_const(col)), static_cast<std::size_t>(n_rows)};
}

auto get_vals(SCIP_COL const* col) noexcept -> nonstd::span<SCIP_Real> {
	auto const n_rows = SCIPcolGetNNonz(not_const(col));
	return {SCIPcolGetVals(not_const(col)), static_cast<std::size_t>(n_rows)};
}

}  // namespace ecole::scip
