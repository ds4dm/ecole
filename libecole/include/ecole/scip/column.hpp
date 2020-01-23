#pragma once

#include "ecole/scip/variable.hpp"
#include "ecole/scip/view.hpp"

// Avoid including SCIP header
typedef struct SCIP_Col SCIP_Col;

namespace ecole {
namespace scip {

class ColProxy : public Proxy<SCIP_Col> {
public:
	using Proxy::Proxy;

	double ub() const noexcept;
	double lb() const noexcept;
	double reduced_cost() const noexcept;

	VarProxy var() const noexcept;
};

using ColView = View<ColProxy>;

}  // namespace scip
}  // namespace ecole
