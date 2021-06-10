#include <chrono>

#include "scip/scip.h"
#include "scip/type_event.h"
#include <objscip/objeventhdlr.h>

#include "ecole/reward/boundintegral.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

/*
 * Helper functions for IntegralEventHandler
 */

auto time_now(bool wall) -> std::chrono::nanoseconds {
	if (wall) {
		return std::chrono::steady_clock::now().time_since_epoch();
	}
	return utility::cpu_clock::now().time_since_epoch();
}

/* Gets the primal bound of the scip model */
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

/* Gets the dual bound of the scip model */
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

auto is_lp_event(SCIP_EVENTTYPE event) {

	if ((event == SCIP_EVENTTYPE_LPEVENT) | 
		(event == SCIP_EVENTTYPE_FIRSTLPSOLVED) |
		(event == SCIP_EVENTTYPE_LPSOLVED)) {
		return true;
	}
	return false;
}

auto is_bestsol_event(SCIP_EVENTTYPE event) {
	if (event == SCIP_EVENTTYPE_BESTSOLFOUND) {
		return true;
	}
	return false;
}

/*
 * Class definition for IntegralEventHandler
 */

class IntegralEventHandler : public ::scip::ObjEventhdlr {
public:
	IntegralEventHandler(SCIP* scip, bool wall_, bool extract_primal_, bool extract_dual_) :
		ObjEventhdlr(scip, "ecole::reward::IntegralEventHandler", "Event handler for primal and dual integrals"), 
		wall{wall_},
		extract_primal{extract_primal_},
		extract_dual{extract_dual_} {} 
	~IntegralEventHandler() override = default;
	SCIP_DECL_EVENTINIT(scip_init) override;
	SCIP_DECL_EVENTEXIT(scip_exit) override;
	SCIP_DECL_EVENTEXEC(scip_exec) override;
	
	[[nodiscard]] std::vector<std::chrono::nanoseconds> const& get_times() const noexcept { return times; }
	[[nodiscard]] std::vector<SCIP_Real> const& get_primal_bounds() const noexcept { return primal_bounds; }
	[[nodiscard]] std::vector<SCIP_Real> const& get_dual_bounds() const noexcept { return dual_bounds; }

	void extract_metrics(SCIP* scip, SCIP_EVENTTYPE event_type = 0);

private:
	bool wall;
	bool extract_primal;
	bool extract_dual;
	std::vector<std::chrono::nanoseconds> times;
	std::vector<SCIP_Real> primal_bounds;
	std::vector<SCIP_Real> dual_bounds;
};

/*
 * Methods for IntegralEventHandler
 */

/* Catches primal and dual related events */
SCIP_DECL_EVENTINIT(IntegralEventHandler::scip_init) {
	if (extract_primal) {
		SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, nullptr));
	}
	if (extract_dual) {
		SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, nullptr));
	}
	return SCIP_OKAY;
}

/* Drops primal and dual related events */
SCIP_DECL_EVENTEXIT(IntegralEventHandler::scip_exit) {
	if (extract_primal) {
		SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, -1));
	}
	if (extract_dual) {
		SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, -1));
	}
	return SCIP_OKAY;
}

/* Calls extract_metrics() to obtain bounds/times at events */
SCIP_DECL_EVENTEXEC(IntegralEventHandler::scip_exec) {
	extract_metrics(scip_, SCIPeventGetType(event));
	return SCIP_OKAY;
}

/* Gets and adds primal/dual bounds and times to vectors */
void IntegralEventHandler::extract_metrics(SCIP* scip, SCIP_EVENTTYPE event_type) {
	if (extract_primal) {
		if ((is_lp_event(event_type)) | (primal_bounds.size() == 0)) {
			primal_bounds.push_back(get_primal_bound(scip));
		} else {
			primal_bounds.push_back(primal_bounds[primal_bounds.size()-1]);
		}
	}
	if (extract_dual) {
		if ((is_bestsol_event(event_type)) | (dual_bounds.size() == 0)) {
			dual_bounds.push_back(get_dual_bound(scip));
		} else {
			dual_bounds.push_back(dual_bounds[dual_bounds.size()-1]);
		}
	}
	times.push_back(time_now(scip));
}


/*
 * Helper functions for BoundIntegral
 */

auto compute_dual_integral(
	std::vector<SCIP_Real> const & dual_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real dual_integral = 0.0;
	for (size_t i = 0; i < dual_bounds.size() - 1; ++i) {
		auto const db = std::max(dual_bounds[i], initial_dual_bound);
		auto const dual_bound_diff = initial_primal_bound - db;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		dual_integral += dual_bound_diff * time_diff;
	}

	return dual_integral;
}

auto compute_primal_integral(
	std::vector<SCIP_Real> const & primal_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
	SCIP_Real primal_integral = 0.0;
	for (size_t i = 0; i < primal_bounds.size() - 1; ++i) {
		auto const pb = std::min(primal_bounds[i], initial_primal_bound);
		auto const primal_bound_diff = pb - initial_dual_bound;
		auto const time_diff = std::chrono::duration<double>(times[i + 1] - times[i]).count();
		primal_integral += primal_bound_diff * time_diff;
	}

	return primal_integral;
}

auto compute_primal_dual_integral(
	std::vector<SCIP_Real> const & primal_bounds,
	std::vector<SCIP_Real> const & dual_bounds,
	std::vector<std::chrono::nanoseconds> const & times,
	SCIP_Real const initial_primal_bound,
	SCIP_Real const initial_dual_bound) {
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

/* Returns the integral event handler */
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

template <Bound bound> 
BoundIntegral<bound>::BoundIntegral(
	bool wall_,
	BoundFunction bound_function_) {
	wall = wall_;
	if (bound_function_) {
		bound_function = bound_function_;
	} else {
		bound_function = default_bound_function;
	}
}

template <Bound bound>
void BoundIntegral<bound>::before_reset(scip::Model& model) {
	last_integral = 0.0;

	/* Get bounds for computing primal-dual integral on instance */
	std::tie(initial_dual_bound, initial_primal_bound) = bound_function(model);

	/* Initalize and add event handler */
	if constexpr(bound == Bound::dual) {
		SCIPincludeObjEventhdlr(model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, FALSE, TRUE), TRUE);
    } else if constexpr(bound == Bound::primal) {
    	SCIPincludeObjEventhdlr(model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, FALSE), TRUE);
    } else if constexpr(bound == Bound::primal_dual) {
    	SCIPincludeObjEventhdlr(model.get_scip_ptr(), new IntegralEventHandler(model.get_scip_ptr(), wall, TRUE, TRUE), TRUE);
    }

	/* Extract metrics before resetting to get initial reference point */ 
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());
}

template <Bound bound>
Reward BoundIntegral<bound>::extract(scip::Model& model, bool /*done*/) {

	/* Get info from event handler */
	auto handler = get_eventhdlr(model);
	handler->extract_metrics(model.get_scip_ptr());

	auto const dual_bounds = handler->get_dual_bounds();
	auto const primal_bounds = handler->get_primal_bounds();
	auto const times = handler->get_times();

	/* Compute primal integral and difference */
	SCIP_Real integral;
	if constexpr(bound == Bound::dual) {
		integral =  compute_dual_integral(
			dual_bounds, 
			times,
			initial_primal_bound,
			initial_dual_bound);
    } else if constexpr(bound == Bound::primal) {
    	integral =  compute_primal_integral(
    		primal_bounds,
			times,
			initial_primal_bound,
			initial_dual_bound);
    } else if constexpr(bound == Bound::primal_dual) {
    	integral =  compute_primal_dual_integral(
    		primal_bounds, 
			dual_bounds, 
			times,
			initial_primal_bound,
			initial_dual_bound);
    }
	
	/* Compute diff and update last_integral */
	auto const integral_diff = integral - last_integral;
	last_integral = integral;

	return static_cast<Reward>(integral_diff);
}

template class BoundIntegral<Bound::primal>; 
template class BoundIntegral<Bound::dual>; 
template class BoundIntegral<Bound::primal_dual>; 

}  // namespace ecole::reward
