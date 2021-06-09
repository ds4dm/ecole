#pragma once

#include <chrono>
#include <vector>

#include <objscip/objeventhdlr.h>

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

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

}  // namespace ecole::reward
