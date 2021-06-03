#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/dualintegral.hpp"
#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_dual_integral(
	std::vector<SCIP_Real> dual_bounds,
	std::vector<std::chrono::nanoseconds> times,
	SCIP_Real dual_bound_reference) {
	SCIP_Real dual_integral = 0.0;
	for (size_t i = 0; i < dual_bounds.size() - 1; ++i) {
		auto const dual_bound_diff = dual_bound_reference - dual_bounds[i];
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		dual_integral += dual_bound_diff * time_diff;
	}

	return dual_integral;
}

/* Compute the dual bounds adjusted by the initial bound given. */
auto get_adjusted_dual_bounds(std::vector<SCIP_Real> dual_bounds, SCIP_Real initial_dual_bound) {
	std::vector<SCIP_Real> adjusted_dual_bounds(dual_bounds.size());
	for (std::size_t i = 0; i < dual_bounds.size(); ++i) {
		if (dual_bounds[i] < initial_dual_bound) {
			adjusted_dual_bounds[i] = initial_dual_bound;
		} else {
			adjusted_dual_bounds[i] = dual_bounds[i];
		}
	}
	return adjusted_dual_bounds;
}

/* Default function for returning +/-infinity for the bounds in computing dual integral */
std::tuple<SCIP_Real, SCIP_Real> default_bound_function(scip::Model& model) {
	return std::make_tuple(-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr()));
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
	auto [lb, ub] = bound_function(model);
	dual_bound_reference = ub;
	initial_dual_bound = lb;

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(
		model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, FALSE, TRUE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward DualIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler->get_dual_bounds();
	auto const times = handler->get_times();

	/* Compute dual integral and difference */
	auto const adjusted_dual_bounds = get_adjusted_dual_bounds(dual_bounds, initial_dual_bound);
	auto const dual_integral = compute_dual_integral(adjusted_dual_bounds, times, dual_bound_reference);
	auto const dual_integral_diff = dual_integral - last_dual_integral;

	/* Update last_dual_integral */
	last_dual_integral = dual_integral;

	return static_cast<Reward>(dual_integral_diff);
}

}  // namespace ecole::reward
