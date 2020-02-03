#pragma once

#include <nonstd/optional.hpp>
#include <scip/scip.h>

#include "ecole/scip/type.hpp"
#include "ecole/scip/view.hpp"

namespace ecole {
namespace scip {

using nonstd::optional;

using var_type = SCIP_Vartype;
template <> struct enum_size<var_type> { static constexpr std::size_t value = 4; };

class VarProxy : public Proxy<SCIP_Var> {
public:
	using Proxy::Proxy;

	static VarProxy const None;

	optional<real> ub_local() const noexcept;
	optional<real> lb_local() const noexcept;
	optional<real> best_sol_val() const noexcept;
	optional<real> avg_sol() const noexcept;
	var_type type_() const noexcept;

	friend class Model;
};

using VarView = View<VarProxy>;

}  // namespace scip
}  // namespace ecole
