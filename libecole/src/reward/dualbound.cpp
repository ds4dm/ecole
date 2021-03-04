#include "ecole/reward/dualbound.hpp"
#include "ecole/scip/model.hpp"

#define infinity   1e+20

namespace ecole::reward {

static auto dual_bound(scip::Model& model) {
	switch (model.get_stage()) {
	// Only stages when the following call is authorized
	case SCIP_STAGE_TRANSFORMED:
  case SCIP_STAGE_INITPRESOLVE:
  case SCIP_STAGE_PRESOLVING:
  case SCIP_STAGE_EXITPRESOLVE:
  case SCIP_STAGE_PRESOLVED:
  case SCIP_STAGE_INITSOLVE:
  case SCIP_STAGE_SOLVING:
  case SCIP_STAGE_SOLVED:
		return SCIPgetDualbound(model.get_scip_ptr());
	default:
		return decltype(SCIPgetDualbound(nullptr)){0};
	}
}

void DualBound::before_reset(scip::Model& /*Model*/) {
  	dual_bound_value = - infinity;
}

Reward DualBound::extract(scip::Model& model, bool /* done */) {
	auto dual_bound_value = dual_bound(model);
	return static_cast<double>(dual_bound_value);
}

}  // namespace ecole::reward