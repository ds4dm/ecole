#pragma once

#include <nonstd/optional.hpp>

#include "ecole/scip/type.hpp"
#include "ecole/scip/variable.hpp"
#include "ecole/scip/view.hpp"

// Avoid including SCIP header
typedef struct SCIP_Col SCIP_Col;

namespace ecole {
namespace scip {

using nonstd::optional;

class ColProxy : public Proxy<SCIP_Col> {
public:
	using Proxy::Proxy;

	optional<real> ub() const noexcept;
	optional<real> lb() const noexcept;
	real reduced_cost() const noexcept;
	real obj() const noexcept;

	VarProxy var() const noexcept;
};

using ColView = View<ColProxy>;

}  // namespace scip
}  // namespace ecole
