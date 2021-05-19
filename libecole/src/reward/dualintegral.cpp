#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/dualintegral_eventhdlr.hpp"
#include "ecole/reward/dualintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"


namespace ecole::reward {

namespace {

/* Gets the time now */
auto time_now(bool wall) -> std::chrono::nanoseconds {
	if (wall) {
		return std::chrono::steady_clock::now().time_since_epoch();
	}
	return utility::cpu_clock::now().time_since_epoch();
}

/* Compute the dual integral */
auto compute_dual_integral(std::vector<scip::real> dual_bounds, 
	std::vector<std::chrono::nanoseconds> times,
	std::chrono::nanoseconds now,
	scip::real dual_bound_reference) {

	/* Initial primal integral before no solution is found. */
	scip::real dual_integral = 0.0;

	for (size_t i = 0; i < dual_bounds.size() - 1; ++i) {
		auto const dual_bound_diff = dual_bound_reference - dual_bounds[i];
		auto const time_diff = std::chrono::duration<double>(times[i+1] - times[i]).count();
		dual_integral += dual_bound_diff * time_diff;
	}

	/* Dual integral between current time and last bound. */
	auto const dual_bound_diff = dual_bound_reference - dual_bounds[dual_bounds.size() - 1];
	auto const time_diff = std::chrono::duration<double>(now - times[dual_bounds.size() - 1]).count();
	dual_integral += dual_bound_diff * time_diff;

	return dual_integral;
}

/* Compute the dual bounds adjusted by the initial bound given. */
auto get_adjusted_dual_bounds(std::vector<scip::real> dual_bounds, scip::real initial_dual_bound) {
	std::vector<scip::real> adjusted_dual_bounds(dual_bounds.size());
	for (std::size_t i = 0; i < dual_bounds.size(); ++i) {
		if (dual_bounds[i] < initial_dual_bound) {
			adjusted_dual_bounds[i] = initial_dual_bound;
		} else {
			adjusted_dual_bounds[i] = dual_bounds[i];
		}
	}
	return adjusted_dual_bounds;
}

} // namespace


/*********************
 Dual Integral Methods
**********************/

/* */
void DualIntegral::before_reset(scip::Model& model) {
  	last_dual_intgral = 0.0;

  	// These are the values we need to figure out how to set
  	// for each instance.  
	dual_bound_reference = SCIPinfinity(model.get_scip_ptr());
	initial_dual_bound = - SCIPinfinity(model.get_scip_ptr());

	/* Initalize and add event handler */
	eventhdlr = new DualIntegralEventHandler(model.get_scip_ptr(), wall);
    SCIPincludeObjEventhdlr(model.get_scip_ptr(), eventhdlr, TRUE);

    /* Extract metrics before resetting to get initial reference point */
    eventhdlr->extract_metrics();
    
}

/* */
Reward DualIntegral::extract(scip::Model& /*model*/, bool /*done*/) {
	/* Get info from event handler */
	auto const dual_bounds = eventhdlr->get_dual_bounds();
	auto const times = eventhdlr->get_times();
	auto const now = time_now(wall);

	/* Compute dual integral and difference */
	auto const adjusted_dual_bounds = get_adjusted_dual_bounds(dual_bounds, initial_dual_bound);
	auto const dual_integral = compute_dual_integral(adjusted_dual_bounds, times, now, dual_bound_reference);
	auto const dual_integral_diff = dual_integral - last_dual_intgral;
	
	/* Update last_dual_integral */
	last_dual_intgral = dual_integral;

	return static_cast<Reward>(dual_integral_diff);
}

}  // namespace ecole::reward
