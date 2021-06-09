#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/dualintegral.hpp"
#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_dual_integral(
	std::vector<SCIP_Real> const & dual_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const dual_bound_reference,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real dual_integral = 0.0;
	for (size_t i = 0; i < dual_bounds.size() - 1; ++i) {
		auto const db = std::max(dual_bounds[i], initial_dual_bound);
		auto const dual_bound_diff = dual_bound_reference - db;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		dual_integral += dual_bound_diff * time_diff;
	}

	return dual_integral;
}

/* Returns the integral event handler and check that it exists.  */
auto get_eventhdlr(scip::Model& model) {
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	return handler;
}

/* Default function for returning +/-infinity for the bounds in computing dual integral */
std::tuple<SCIP_Real, SCIP_Real> default_bound_function(scip::Model& model) {
	return {-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr())};
}

}  // namespace

DualIntegral::DualIntegral(
	bool wall_,
	std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function_) {
	wall = wall_;
	if (bound_function_) {
		bound_function = bound_function_;
	} else {
		bound_function = default_bound_function;
	}
}

void DualIntegral::before_reset(scip::Model& model) {
	last_dual_integral = 0.0;

	/* Get bounds for computing dual integral on instance */
	std::tie(initial_dual_bound, dual_bound_reference) = bound_function(model);

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(
		model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, FALSE, TRUE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward DualIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler->get_dual_bounds();
	auto const times = handler->get_times();

	/* Compute dual integral and difference */
	auto const dual_integral = compute_dual_integral(dual_bounds, 
		times, 
		dual_bound_reference,
		initial_dual_bound);
	auto const dual_integral_diff = dual_integral - last_dual_integral;

	/* Update last_dual_integral */
	last_dual_integral = dual_integral;

	return static_cast<Reward>(dual_integral_diff);
}

}  // namespace ecole::reward
