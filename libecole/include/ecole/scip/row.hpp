#pragma once

#include <nonstd/optional.hpp>
#include <scip/scip.h>

#include "ecole/scip/type.hpp"
#include "ecole/scip/view.hpp"

namespace ecole {
namespace scip {

using nonstd::optional;

class RowProxy : public Proxy<SCIP_Row> {
public:
	using Proxy::Proxy;

	real constant() const noexcept;
	optional<real> lhs() const noexcept;
	optional<real> rhs() const noexcept;
	int n_lp_nonz() const noexcept;
	real l2_norm() const noexcept;
	real obj_cos_sim() const noexcept;

	bool is_local() const noexcept;
	bool is_modifiable() const noexcept;
	bool is_removable() const noexcept;

	real dual_sol() const noexcept;
	base_stat basis_status() const noexcept;
	int age() const noexcept;
	real lp_activity() const noexcept;
	bool is_at_lhs() const noexcept;
	bool is_at_rhs() const noexcept;
};

using RowView = View<RowProxy>;

}  // namespace scip
}  // namespace ecole
