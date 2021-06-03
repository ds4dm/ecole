#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/reward/primalintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_primal_integral(
	std::vector<SCIP_Real> primal_bounds,
	std::vector<std::chrono::nanoseconds> times,
	SCIP_Real primal_bound_reference) {
	SCIP_Real primal_integral = 0.0;
	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const primal_bound_diff = primal_bounds[i] - primal_bound_reference;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_integral += primal_bound_diff * time_diff;
	}

	return primal_integral;
}

/* Compute the primal bounds adjusted by the initial bound given. */
auto get_adjusted_primal_bounds(std::vector<SCIP_Real> primal_bounds, SCIP_Real initial_primal_bound) {
	std::vector<SCIP_Real> adjusted_primal_bounds(primal_bounds.size());
	for (std::size_t i = 0; i < primal_bounds.size(); ++i) {
		if (primal_bounds[i] > initial_primal_bound) {
			adjusted_primal_bounds[i] = initial_primal_bound;
		} else {
			adjusted_primal_bounds[i] = primal_bounds[i];
		}
	}
	return adjusted_primal_bounds;
}

/* Default function for returning +/-infinity for the bounds in computing primal integral */
std::tuple<SCIP_Real, SCIP_Real> default_bound_function(scip::Model& model) {
	return std::make_tuple(-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr()));
}

}  // namespace

PrimalIntegral::PrimalIntegral(
	bool wall_,
	std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function_) {
	wall = wall_;
	if (bound_function_) {
		bound_function = bound_function_;
	} else {
		bound_function = default_bound_function;
	}
}

void PrimalIntegral::before_reset(scip::Model& model) {
	last_primal_integral = 0.0;

	/* Get bounds for computing primal integral on instance */
	auto [lb, ub] = bound_function(model);
	initial_primal_bound = ub;
	primal_bound_reference = lb;

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(
		model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, FALSE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward PrimalIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());

	auto const primal_bounds = handler->get_primal_bounds();
	auto const times = handler->get_times();

	/* Compute primal integral and difference */
	auto const adjusted_primal_bounds = get_adjusted_primal_bounds(primal_bounds, initial_primal_bound);
	auto const primal_integral = compute_primal_integral(adjusted_primal_bounds, times, primal_bound_reference);
	auto const primal_integral_diff = primal_integral - last_primal_integral;

	/* Update last_primal_integral */
	last_primal_integral = primal_integral;

	return static_cast<Reward>(primal_integral_diff);
}

}  // namespace ecole::reward
