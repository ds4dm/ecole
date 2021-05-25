#pragma once

#include <chrono>
#include <vector>

#include <objscip/objeventhdlr.h>
#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class IntegralEventHandler : public ::scip::ObjEventhdlr {
public:
	IntegralEventHandler(SCIP* scip, bool wall_, bool primal_, bool dual_):
		ObjEventhdlr(scip, "ecole::reward::IntegralEventHandler", "Event handler for primal and dual integrals") {
		wall = wall_;
		primal = primal_;
		dual = dual_;
	}
	~IntegralEventHandler() override = default;
	virtual SCIP_DECL_EVENTINIT(scip_init);
	virtual SCIP_DECL_EVENTEXIT(scip_exit);
	virtual SCIP_DECL_EVENTEXEC(scip_exec);

	std::vector<std::chrono::nanoseconds> get_times();
	std::vector<SCIP_Real> get_primal_bounds();
	std::vector<SCIP_Real> get_dual_bounds();
	void extract_metrics(SCIP * scip);

private:
	bool wall;
	bool primal;
	bool dual;
	std::vector<std::chrono::nanoseconds> times;
	std::vector<SCIP_Real> primal_bounds;
	std::vector<SCIP_Real> dual_bounds;
};

}  // namespace ecole::reward
