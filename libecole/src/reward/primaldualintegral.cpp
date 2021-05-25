#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/reward/primaldualintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto compute_primal_dual_integral(
	std::vector<SCIP_Real> primal_bounds,
	std::vector<SCIP_Real> dual_bounds,
	std::vector<std::chrono::nanoseconds> times) {
	SCIP_Real primal_dual_integral = 0.0;
	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const primal_dual_bound_diff = primal_bounds[i] - dual_bounds[i];
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_dual_integral += primal_dual_bound_diff * time_diff;
	}

	return primal_dual_integral;
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

}  // namespace

void PrimalDualIntegral::before_reset(scip::Model& model) {
	last_primal_dual_integral = 0.0;

	// These are the values we need to figure out how to set
	// for each instance.
	initial_primal_bound = SCIPinfinity(model.get_scip_ptr());
	initial_dual_bound = -SCIPinfinity(model.get_scip_ptr());

	/* Initalize and add event handler */
	SCIPincludeObjEventhdlr(model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, TRUE), TRUE);

	/* Extract metrics before resetting to get initial reference point */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());
}

Reward PrimalDualIntegral::extract(scip::Model& model, bool /*done*/) {
	/* Get info from event handler */
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), "ecole::reward::IntegralEventHandler");
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	handler->extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler->get_dual_bounds();
	auto const primal_bounds = handler->get_primal_bounds();
	auto const times = handler->get_times();

	/* Compute primal integral and difference */
	auto const adjusted_primal_bounds = get_adjusted_primal_bounds(primal_bounds, initial_primal_bound);
	auto const adjusted_dual_bounds = get_adjusted_dual_bounds(dual_bounds, initial_dual_bound);
	auto const primal_dual_integral = compute_primal_dual_integral(adjusted_primal_bounds, adjusted_dual_bounds, times);
	auto const primal_dual_integral_diff = primal_dual_integral - last_primal_dual_integral;

	/* Update last_primal_dual_integral */
	last_primal_dual_integral = primal_dual_integral;

	return static_cast<Reward>(primal_dual_integral_diff);
}

}  // namespace ecole::reward
