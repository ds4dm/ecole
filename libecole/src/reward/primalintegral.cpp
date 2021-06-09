#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/reward/primalintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_primal_integral(
	std::vector<SCIP_Real> const & primal_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const primal_bound_reference) {
	SCIP_Real primal_integral = 0.0;
	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const pb = std::min(primal_bounds[i], initial_primal_bound);
		auto const primal_bound_diff = pb - primal_bound_reference;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_integral += primal_bound_diff * time_diff;
	}

	return primal_integral;
}

/* Returns the integral event handler and check that it exists.  */
auto get_eventhdlr(scip::Model& model) {
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	return handler;
}

/* Default function for returning +/-infinity for the bounds in computing primal integral */
std::tuple<SCIP_Real, SCIP_Real> default_bound_function(scip::Model& model) {
	return {-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr())};
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
	std::tie(primal_bound_reference, initial_primal_bound) = bound_function(model);

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(
		model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, FALSE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward PrimalIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());

	auto const primal_bounds = handler->get_primal_bounds();
	auto const times = handler->get_times();

	/* Compute primal integral and difference */
	auto const primal_integral = compute_primal_integral(primal_bounds, 
		times, 
		primal_bound_reference,
		initial_primal_bound);
	auto const primal_integral_diff = primal_integral - last_primal_integral;

	/* Update last_primal_integral */
	last_primal_integral = primal_integral;

	return static_cast<Reward>(primal_integral_diff);
}

}  // namespace ecole::reward
