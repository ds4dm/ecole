#pragma once

#include "ecole/scip/view.hpp"

// Avoid including SCIP header
typedef struct SCIP_Var SCIP_Var;

namespace ecole {
namespace scip {

class Model;

class VarProxy : public Proxy<SCIP_Var> {
public:
	VarProxy(SCIP_Var* value) noexcept : Proxy(value) {}

	double ub() const noexcept;

	friend class Model;
};

using VarView = View<SCIP_Var, VarProxy>;

} // namespace scip
} // namespace ecole
