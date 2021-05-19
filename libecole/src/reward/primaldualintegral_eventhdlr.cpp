#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/primaldualintegral_eventhdlr.hpp"
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

/* Gets the primal bound of the scip model */
auto get_primal_bound(SCIP* scip_) {
	switch (SCIPgetStage(scip_)) {
	case SCIP_STAGE_TRANSFORMED:
	case SCIP_STAGE_INITPRESOLVE:
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_EXITPRESOLVE:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_INITSOLVE:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetPrimalbound(scip_);
	default:
		return SCIPinfinity(scip_);
	}
}

/* Gets the dual bound of the scip model */
auto get_dual_bound(SCIP* scip_) {
	switch (SCIPgetStage(scip_)) {
	case SCIP_STAGE_TRANSFORMED:
	case SCIP_STAGE_INITPRESOLVE:
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_EXITPRESOLVE:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_INITSOLVE:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetDualbound(scip_);
	default:
		return -SCIPinfinity(scip_);
	}
}

}  // namespace

/*********************
 Event Handler Methods
**********************/

/* */
SCIP_DECL_EVENTFREE(PrimalDualIntegralEventHandler::scip_free) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTDELETE(PrimalDualIntegralEventHandler::scip_delete) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTINIT(PrimalDualIntegralEventHandler::scip_init) {
	SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, nullptr));
	SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, nullptr));
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXIT(PrimalDualIntegralEventHandler::scip_exit) {
	SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_BESTSOLFOUND, eventhdlr, nullptr, -1));
	SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, -1));
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTINITSOL(PrimalDualIntegralEventHandler::scip_initsol) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXITSOL(PrimalDualIntegralEventHandler::scip_exitsol) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXEC(PrimalDualIntegralEventHandler::scip_exec) {
	extract_metrics();
	return SCIP_OKAY;
}

/* Gets and adds primal/dual bounds and times to vectors */

void PrimalDualIntegralEventHandler::extract_metrics() {
	// TODO: Handle each event individually
	auto const primal_bound = get_primal_bound(scip_);
	auto const dual_bound = get_dual_bound(scip_);
	auto const time = time_now(wall);
	primal_bounds.push_back(primal_bound);
	dual_bounds.push_back(dual_bound);
	times.push_back(time);
}

/* Returns the vector of times */
std::vector<std::chrono::nanoseconds> PrimalDualIntegralEventHandler::get_times() {
	return times;
}

/* Returns the vector of primal bounds */
std::vector<scip::real> PrimalDualIntegralEventHandler::get_primal_bounds() {
	return primal_bounds;
}

/* Returns the vector of dual bounds */
std::vector<scip::real> PrimalDualIntegralEventHandler::get_dual_bounds() {
	return dual_bounds;
}

}  // namespace ecole::reward
