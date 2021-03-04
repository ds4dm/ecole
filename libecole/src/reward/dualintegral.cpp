#include "ecole/reward/dualintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace{
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
} // namespace

namespace {
  auto time_now(bool wall) -> std::chrono::nanoseconds {
  	if (wall) {
  		return std::chrono::steady_clock::now().time_since_epoch();
 	 }
  	return utility::cpu_clock::now().time_since_epoch();
  }
}  // namespace


void DualIntegral::before_reset(scip::Model& /*Model*/) {
  	last_dual_bound = 0; //to change for initial value ou -infinity
    	solving_time_offset = time_now(wall);
}

Reward DualIntegral::extract(scip::Model& model, bool /* done */) {
	auto const now = time_now(wall);
	// Casting to seconds represented as a Reward (no ratio).
	auto const solving_time_diff = std::chrono::duration<Reward>{now - solving_time_offset}.count();
	solving_time_offset = now;
  	auto dual_integral = 1/2 * (dual_bound(model) + last_dual_bound)  * solving_time_diff;
 	last_dual_bound = dual_bound(model);
	return static_cast<double>(dual_integral);
}

}  // namespace ecole::reward
