#pragma once

#include <chrono>
#include <vector>

#include <objscip/objeventhdlr.h>
#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"


namespace ecole::reward {

class DualIntegralEventHandler : public ::scip::ObjEventhdlr {
public:
   /* Methods */
	DualIntegralEventHandler(SCIP* scip, bool wall_) : ObjEventhdlr(scip, "dual_integral","event handler for dual integral") {wall = wall_;}
	virtual ~DualIntegralEventHandler() {}
	virtual SCIP_DECL_EVENTFREE(scip_free);
	virtual SCIP_DECL_EVENTINIT(scip_init);
	virtual SCIP_DECL_EVENTEXIT(scip_exit);
	virtual SCIP_DECL_EVENTINITSOL(scip_initsol);
	virtual SCIP_DECL_EVENTEXITSOL(scip_exitsol);
	virtual SCIP_DECL_EVENTDELETE(scip_delete);
	virtual SCIP_DECL_EVENTEXEC(scip_exec);

	std::vector<std::chrono::nanoseconds> get_times();
	std::vector<scip::real> get_dual_bounds();
	void extract_metrics();

private:
	/* Data */
	bool wall;
	std::vector<std::chrono::nanoseconds> times;
	std::vector<scip::real> dual_bounds;

};

} // namespace