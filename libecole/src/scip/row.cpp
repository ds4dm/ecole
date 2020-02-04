#include "ecole/scip/row.hpp"

namespace ecole {
namespace scip {

optional<real> RowProxy::lhs() const noexcept {
	auto const lhs_val = SCIProwGetLhs(value);
	if (SCIPisInfinity(scip, REALABS(lhs_val)))
		return {};
	else
		return lhs_val;
}

optional<real> RowProxy::rhs() const noexcept {
	auto const rhs_val = SCIProwGetRhs(value);
	if (SCIPisInfinity(scip, REALABS(rhs_val)))
		return {};
	else
		return rhs_val;
}

}  // namespace scip
}  // namespace ecole
