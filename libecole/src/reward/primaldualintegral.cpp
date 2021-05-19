#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/primaldualintegral_eventhdlr.hpp"
#include "ecole/reward/primaldualintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

#include <iostream> // REMOVE

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
auto compute_primal_dual_integral(std::vector<scip::real> primal_bounds, 
	std::vector<scip::real> dual_bounds, 
	std::vector<std::chrono::nanoseconds> times,
	std::chrono::nanoseconds now) {

	/* Primal integrals updated when each new bound is found. */
	scip::real primal_dual_integral = 0.0;

	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const primal_dual_bound_diff = primal_bounds[i] - dual_bounds[i];
		auto const time_diff = std::chrono::duration<double>(times[i+1] - times[i]).count();
		primal_dual_integral += primal_dual_bound_diff * time_diff;
	}

	/* Primal integral between current time and last bound. */
	auto const primal_dual_bound_diff = primal_bounds[primal_bounds.size() - 1] - dual_bounds[dual_bounds.size() - 1];
	auto const time_diff = std::chrono::duration<double>(now - times[primal_bounds.size() - 1]).count();
	primal_dual_integral += primal_dual_bound_diff * time_diff;

	return primal_dual_integral;
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


/***********************
 Primal Integral Methods
************************/

/* */
void PrimalDualIntegral::before_reset(scip::Model& model) {
  	last_primal_dual_intgral = 0.0;

  	// These are the values we need to figure out how to set
  	// for each instance.  
  	initial_primal_bound = SCIPinfinity(model.get_scip_ptr());
	initial_dual_bound = - SCIPinfinity(model.get_scip_ptr());

	/* Initalize and add event handler */
	eventhdlr = new PrimalDualIntegralEventHandler(model.get_scip_ptr(), wall);
    SCIPincludeObjEventhdlr(model.get_scip_ptr(), eventhdlr, TRUE);
    
    /* Extract metrics before resetting to get initial reference point */
    eventhdlr->extract_metrics();
}

/* */
Reward PrimalDualIntegral::extract(scip::Model& /*model*/, bool /*done*/) {
	/* Get info from event handler */
	auto const primal_bounds = eventhdlr->get_primal_bounds();
	auto const dual_bounds = eventhdlr->get_dual_bounds();
	auto const times = eventhdlr->get_times();
	auto const now = time_now(wall); 

	
	std::cout << "Dual Bounds:" << std::endl;
	for(std::size_t i = 0; i < dual_bounds.size(); ++i) {
		std::cout << dual_bounds[i] << ", ";
	} std::cout << std::endl;

	std::cout << "Primal Bounds:" << std::endl;
	for(std::size_t i = 0; i < dual_bounds.size(); ++i) {
		std::cout << primal_bounds[i] << ", ";
	} std::cout << std::endl;
	
	/* Compute primal integral and difference */
	auto const adjusted_primal_bounds = get_adjusted_primal_bounds(primal_bounds, initial_primal_bound);
	auto const adjust_dual_bounds = get_adjusted_dual_bounds(dual_bounds, initial_dual_bound);
	auto const primal_dual_integral = compute_primal_dual_integral(adjusted_primal_bounds, adjust_dual_bounds, times, now);
	auto const primal_dual_integral_diff = primal_dual_integral - last_primal_dual_intgral;
	
	/* Update last_primal_dual_intgral */
	last_primal_dual_intgral = primal_dual_integral;

	return static_cast<Reward>(primal_dual_integral_diff);
}

}  // namespace ecole::reward
