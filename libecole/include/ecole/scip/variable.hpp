#pragma once

#include <nonstd/optional.hpp>

#include "ecole/scip/type.hpp"
#include "ecole/scip/view.hpp"

// Avoid including SCIP header
typedef struct SCIP_Var SCIP_Var;

namespace ecole {
namespace scip {

using nonstd::optional;

class VarProxy : public Proxy<SCIP_Var> {
public:
	using Proxy::Proxy;

	static VarProxy const None;

	optional<real> ub_local() const noexcept;
	optional<real> lb_local() const noexcept;
	var_type type_() const noexcept;

	friend class Model;
};

using VarView = View<VarProxy>;

}  // namespace scip
}  // namespace ecole
