#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/primalintegral_eventhdlr.hpp"
#include "ecole/reward/primalintegral.hpp"
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

/* Compute the primal integral */
auto compute_primal_integral(std::vector<scip::real> primal_bounds, 
	std::vector<std::chrono::nanoseconds> times,
	std::chrono::nanoseconds now,
	scip::real primal_bound_reference) {

	/* Primal integrals updated when each new bound is found. */
	scip::real primal_integral = 0.0;

	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const primal_bound_diff = primal_bounds[i] - primal_bound_reference;
		auto const time_diff = std::chrono::duration<double>(times[i+1] - times[i]).count();
		primal_integral += primal_bound_diff * time_diff;
	}

	/* Primal integral between current time and last bound. */
	auto const primal_bound_diff = primal_bounds[primal_bounds.size() - 1] - primal_bound_reference;
	auto const time_diff = std::chrono::duration<double>(now - times[primal_bounds.size() - 1]).count();
	primal_integral += primal_bound_diff * time_diff;

	return primal_integral;
}

/* Compute the primal bounds adjusted by the initial bound given. */
auto get_adjusted_primal_bounds(std::vector<scip::real> primal_bounds, scip::real initial_primal_bound) {
	std::vector<scip::real> adjusted_primal_bounds(primal_bounds.size());
	for (std::size_t i = 0; i < primal_bounds.size(); ++i) {
		if (primal_bounds[i] > initial_primal_bound) {
			adjusted_primal_bounds[i] = initial_primal_bound;
		} else {
			adjusted_primal_bounds[i] = primal_bounds[i];
		}
	}
	return adjusted_primal_bounds;
}

} // namespace


/***********************
 Primal Integral Methods
************************/

/* */
void PrimalIntegral::before_reset(scip::Model& model) {
  	last_primal_intgral = 0.0;

  	// These are the values we need to figure out how to set
  	// for each instance.  
  	initial_primal_bound = SCIPinfinity(model.get_scip_ptr());
  	primal_bound_reference = 0.0;

	/* Initalize and add event handler */
	eventhdlr = new PrimalIntegralEventHandler(model.get_scip_ptr(), wall);
    SCIPincludeObjEventhdlr(model.get_scip_ptr(), eventhdlr, TRUE);

    /* Extract metrics before resetting to get initial reference point */
    eventhdlr->extract_metrics();
    
}

/* */
Reward PrimalIntegral::extract(scip::Model& /*model*/, bool /*done*/) {
	/* Get info from event handler */
	auto const primal_bounds = eventhdlr->get_primal_bounds();
	auto const times = eventhdlr->get_times();
	auto const now = time_now(wall);

	/* Compute primal integral and difference */
	auto const adjusted_primal_bounds = get_adjusted_primal_bounds(primal_bounds, initial_primal_bound);
	auto const primal_integral = compute_primal_integral(adjusted_primal_bounds, times, now, primal_bound_reference);
	auto const primal_integral_diff = primal_integral - last_primal_intgral;
	
	/* Update last_primal_integral */
	last_primal_intgral = primal_integral;
	
	return static_cast<Reward>(primal_integral_diff);
}

}  // namespace ecole::reward
