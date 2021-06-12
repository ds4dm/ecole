#include <chrono>
#include <memory>
#include <vector>

#include "scip/scip.h"
#include "scip/type_event.h"
#include <objscip/objeventhdlr.h>

#include "ecole/reward/boundintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

/*****************************************
 *  Declaration of IntegralEventHanlder  *
 *****************************************/

class IntegralEventHandler : public ::scip::ObjEventhdlr {
public:
	inline static auto constexpr name = "ecole::reward::IntegralEventHandler";

	IntegralEventHandler(SCIP* scip, bool wall_, bool extract_primal_, bool extract_dual_) :
		ObjEventhdlr(scip, name, "Event handler for primal and dual integrals"),
		wall{wall_},
		extract_primal{extract_primal_},
		extract_dual{extract_dual_} {}

	~IntegralEventHandler() override = default;

	[[nodiscard]] std::vector<std::chrono::nanoseconds> const& get_times() const noexcept { return times; }
	[[nodiscard]] std::vector<SCIP_Real> const& get_primal_bounds() const noexcept { return primal_bounds; }
	[[nodiscard]] std::vector<SCIP_Real> const& get_dual_bounds() const noexcept { return dual_bounds; }

	/** Catch primal and dual related events. */
	SCIP_RETCODE scip_init(SCIP* scip, SCIP_EVENTHDLR* eventhdlr) override;
	/** Drop primal and dual related events. */
	SCIP_RETCODE scip_exit(SCIP* scip, SCIP_EVENTHDLR* eventhdlr) override;
	/* Call extract_metrics() to obtain bounds/times at events. */
	SCIP_RETCODE scip_exec(SCIP* scip, SCIP_EVENTHDLR* eventhdlr, SCIP_EVENT* event, SCIP_EVENTDATA* eventdata) override;

	/** Get and adds primal/dual bounds and times to vectors. */
	void extract_metrics(SCIP* scip, SCIP_EVENTTYPE event_type = 0);

private:
	bool wall;
	bool extract_primal;
	bool extract_dual;
	std::vector<std::chrono::nanoseconds> times;
	std::vector<SCIP_Real> primal_bounds;
	std::vector<SCIP_Real> dual_bounds;
};

/********************************************
 *  Implementation of IntegralEventHanlder  *
 ********************************************/

auto IntegralEventHandler::scip_init(SCIP* scip, SCIP_EVENTHDLR* eventhdlr) -> SCIP_RETCODE {
	if (extract_primal) {
		SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, nullptr));
	}
	if (extract_dual) {
		SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, nullptr));
	}
	return SCIP_OKAY;
}

auto IntegralEventHandler::scip_exit(SCIP* scip, SCIP_EVENTHDLR* eventhdlr) -> SCIP_RETCODE {
	if (extract_primal) {
		SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, -1));
	}
	if (extract_dual) {
		SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, -1));
	}
	return SCIP_OKAY;
}

auto IntegralEventHandler::scip_exec(
	SCIP* scip,
	SCIP_EVENTHDLR* /*eventhdlr*/,
	SCIP_EVENT* event,
	SCIP_EVENTDATA* /*eventdata*/) -> SCIP_RETCODE {
	extract_metrics(scip, SCIPeventGetType(event));
	return SCIP_OKAY;
}

/* Get the primal bound of the scip model */
auto get_primal_bound(SCIP* scip) {
	switch (SCIPgetStage(scip)) {
	case SCIP_STAGE_TRANSFORMED:
	case SCIP_STAGE_INITPRESOLVE:
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_EXITPRESOLVE:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_INITSOLVE:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetPrimalbound(scip);
	default:
		return SCIPinfinity(scip);
	}
}

/* Get the dual bound of the scip model */
auto get_dual_bound(SCIP* scip) {
	switch (SCIPgetStage(scip)) {
	case SCIP_STAGE_TRANSFORMED:
	case SCIP_STAGE_INITPRESOLVE:
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_EXITPRESOLVE:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_INITSOLVE:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetDualbound(scip);
	default:
		return -SCIPinfinity(scip);
	}
}

auto time_now(bool wall) -> std::chrono::nanoseconds {
	if (wall) {
		return std::chrono::steady_clock::now().time_since_epoch();
	}
	return utility::cpu_clock::now().time_since_epoch();
}

auto is_lp_event(SCIP_EVENTTYPE event) {
	return (
		(event == SCIP_EVENTTYPE_LPEVENT) || (event == SCIP_EVENTTYPE_FIRSTLPSOLVED) || (event == SCIP_EVENTTYPE_LPSOLVED));
}

auto is_bestsol_event(SCIP_EVENTTYPE event) {
	return event == SCIP_EVENTTYPE_BESTSOLFOUND;
}

void IntegralEventHandler::extract_metrics(SCIP* scip, SCIP_EVENTTYPE event_type) {
	if (extract_primal) {
		if ((is_lp_event(event_type)) || (primal_bounds.empty())) {
			primal_bounds.push_back(get_primal_bound(scip));
		} else {
			primal_bounds.push_back(primal_bounds[primal_bounds.size() - 1]);
		}
	}
	if (extract_dual) {
		if ((is_bestsol_event(event_type)) || (dual_bounds.empty())) {
			dual_bounds.push_back(get_dual_bound(scip));
		} else {
			dual_bounds.push_back(dual_bounds[dual_bounds.size() - 1]);
		}
	}
	times.push_back(time_now(wall));
}

/*************************************
 *  Implementation of BoundIntegral  *
 *************************************/

auto compute_dual_integral(
	std::vector<SCIP_Real> const& dual_bounds,
	std::vector<std::chrono::nanoseconds> const& times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real dual_integral = 0.0;
	for (std::size_t i = 0; i < dual_bounds.size() - 1; ++i) {
		auto const db = std::max(dual_bounds[i], initial_dual_bound);
		auto const dual_bound_diff = initial_primal_bound - db;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		dual_integral += dual_bound_diff * time_diff;
	}

	return dual_integral;
}

auto compute_primal_integral(
	std::vector<SCIP_Real> const& primal_bounds,
	std::vector<std::chrono::nanoseconds> const& times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real primal_integral = 0.0;
	for (std::size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const pb = std::min(primal_bounds[i], initial_primal_bound);
		auto const primal_bound_diff = pb - initial_dual_bound;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_integral += primal_bound_diff * time_diff;
	}

	return primal_integral;
}

auto compute_primal_dual_integral(
	std::vector<SCIP_Real> const& primal_bounds,
	std::vector<SCIP_Real> const& dual_bounds,
	std::vector<std::chrono::nanoseconds> const& times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real primal_dual_integral = 0.0;
	for (std::size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const pb = std::min(primal_bounds[i], initial_primal_bound);
		auto const db = std::max(dual_bounds[i], initial_dual_bound);
		auto const primal_dual_bound_diff = pb - db;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_dual_integral += primal_dual_bound_diff * time_diff;
	}

	return primal_dual_integral;
}

/** Return the integral event handler */
auto get_eventhdlr(scip::Model& model) -> auto& {
	auto* const base_handler = SCIPfindObjEventhdlr(model.get_scip_ptr(), IntegralEventHandler::name);
	assert(base_handler != nullptr);
	auto* const handler = dynamic_cast<IntegralEventHandler*>(base_handler);
	assert(handler != nullptr);
	return *handler;
}

/** Add the integral event handler to the model. */
void add_eventhdlr(scip::Model& model, bool wall, bool extract_primal, bool extract_dual) {
	auto handler = std::make_unique<IntegralEventHandler>(model.get_scip_ptr(), wall, extract_primal, extract_dual);
	scip::call(SCIPincludeObjEventhdlr, model.get_scip_ptr(), handler.get(), true);
	// NOLINTNEXTLINE memory ownership is passed to SCIP
	handler.release();
	// NOLINTNEXTLINE memory ownership is passed to SCIP
}

/** Default function for returning +/-infinity for the bounds in computing primal-dual integral. */
auto default_bound_function(scip::Model& model) -> std::tuple<Reward, Reward> {
	return {-SCIPinfinity(model.get_scip_ptr()), SCIPinfinity(model.get_scip_ptr())};
}

}  // namespace

template <Bound bound>
ecole::reward::BoundIntegral<bound>::BoundIntegral(bool wall_, const BoundFunction& bound_function_) :
	wall{wall_}, bound_function{bound_function_ ? bound_function_ : default_bound_function} {}

template <Bound bound> void BoundIntegral<bound>::before_reset(scip::Model& model) {
	last_integral = 0.0;

	// Get bounds for computing primal-dual integral on instance
	std::tie(initial_dual_bound, initial_primal_bound) = bound_function(model);

	// Initalize and add event handler
	if constexpr (bound == Bound::dual) {
		add_eventhdlr(model, wall, false, true);
	} else if constexpr (bound == Bound::primal) {
		add_eventhdlr(model, wall, true, false);
	} else if constexpr (bound == Bound::primal_dual) {
		add_eventhdlr(model, wall, true, true);
	}

	// Extract metrics before resetting to get initial reference point
	get_eventhdlr(model).extract_metrics(model.get_scip_ptr());
}

template <Bound bound> Reward BoundIntegral<bound>::extract(scip::Model& model, bool /*done*/) {
	// Get info from event handler
	auto& handler = get_eventhdlr(model);
	handler.extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler.get_dual_bounds();
	auto const primal_bounds = handler.get_primal_bounds();
	auto const times = handler.get_times();

	// Compute primal integral and difference
	SCIP_Real integral = 0.;
	if constexpr (bound == Bound::dual) {
		integral = compute_dual_integral(dual_bounds, times, initial_primal_bound, initial_dual_bound);
	} else if constexpr (bound == Bound::primal) {
		integral = compute_primal_integral(primal_bounds, times, initial_primal_bound, initial_dual_bound);
	} else if constexpr (bound == Bound::primal_dual) {
		integral =
			compute_primal_dual_integral(primal_bounds, dual_bounds, times, initial_primal_bound, initial_dual_bound);
	}

	/* Compute diff and update last_integral */
	auto const integral_diff = integral - last_integral;
	last_integral = integral;

	return static_cast<Reward>(integral_diff);
}

template class BoundIntegral<Bound::primal>;
template class ecole::reward::BoundIntegral<Bound::dual>;
template class ecole::reward::BoundIntegral<Bound::primal_dual>;

}  // namespace ecole::reward