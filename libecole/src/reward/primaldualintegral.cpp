#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/reward/primaldualintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_primal_dual_integral(
	std::vector<SCIP_Real> const & primal_bounds,
	std::vector<SCIP_Real> const & dual_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const initial_dual_bound,
	SCIP_Real const initial_primal_bound) {
	SCIP_Real primal_dual_integral = 0.0;
	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const pb = std::min(primal_bounds[i], initial_primal_bound);
		auto const db = std::max(dual_bounds[i], initial_dual_bound);
		auto const primal_dual_bound_diff = pb - db;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_dual_integral += primal_dual_bound_diff * time_diff;
	}

	return primal_dual_integral;
}

/* Returns the integral event handler and check that it exists.  */
auto get_eventhdlr(scip::Model& model) {
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	return handler;
}

/* Default function for returning +/-infinity for the bounds in computing primal-dual integral */
std::tuple<SCIP_Real, SCIP_Real> default_bound_function(scip::Model& model) {
	return {-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr())};
}

}  // namespace

PrimalDualIntegral::PrimalDualIntegral(
	bool wall_,
	const std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function_) {
	wall = wall_;
	if (bound_function_) {
		bound_function = bound_function_;
	} else {
		bound_function = default_bound_function;
	}
}

void PrimalDualIntegral::before_reset(scip::Model& model) {
	last_primal_dual_integral = 0.0;

	/* Get bounds for computing primal-dual integral on instance */
	std::tie(initial_dual_bound, initial_primal_bound) = bound_function(model);

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, TRUE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward PrimalDualIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler->get_dual_bounds();
	auto const primal_bounds = handler->get_primal_bounds();
	auto const times = handler->get_times();

	/* Compute primal integral and difference */
	auto const primal_dual_integral = compute_primal_dual_integral(primal_bounds, 
		dual_bounds, 
		times,
		initial_dual_bound,
		initial_primal_bound);
	auto const primal_dual_integral_diff = primal_dual_integral - last_primal_dual_integral;

	/* Update last_primal_dual_integral */
	last_primal_dual_integral = primal_dual_integral;

	return static_cast<Reward>(primal_dual_integral_diff);
}

}  // namespace ecole::reward
