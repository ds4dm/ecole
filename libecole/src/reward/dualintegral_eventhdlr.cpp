#include "scip/scip.h"
#include "scip/type_event.h"

#include "ecole/reward/dualintegral_eventhdlr.hpp"
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
SCIP_DECL_EVENTFREE(DualIntegralEventHandler::scip_free) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTDELETE(DualIntegralEventHandler::scip_delete) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTINIT(DualIntegralEventHandler::scip_init) {
	SCIP_CALL(SCIPcatchEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, nullptr));
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXIT(DualIntegralEventHandler::scip_exit) {
	SCIP_CALL(SCIPdropEvent(scip, SCIP_EVENTTYPE_LPEVENT, eventhdlr, nullptr, -1));
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTINITSOL(DualIntegralEventHandler::scip_initsol) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXITSOL(DualIntegralEventHandler::scip_exitsol) {
	return SCIP_OKAY;
}

/* */
SCIP_DECL_EVENTEXEC(DualIntegralEventHandler::scip_exec) {
	extract_metrics();
	return SCIP_OKAY;
}

/* Gets and adds dual bounds and times to vectors */
void DualIntegralEventHandler::extract_metrics() {
	auto const dual_bound = get_dual_bound(scip_);
	auto const time = time_now(wall);
	dual_bounds.push_back(dual_bound);
	times.push_back(time);
}

/* Returns the vector of times */
std::vector<std::chrono::nanoseconds> DualIntegralEventHandler::get_times() {
	return times;
}

/* Returns the vector of dual bounds */
std::vector<scip::real> DualIntegralEventHandler::get_dual_bounds() {
	return dual_bounds;
}

}  // namespace ecole::reward
