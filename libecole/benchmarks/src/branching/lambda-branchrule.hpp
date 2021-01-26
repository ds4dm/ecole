#pragma once

#include <objscip/objbranchrule.h>
#include <scip/scip.h>

namespace ecole::scip {

template <typename Func> class LambdaBranchrule : public ::scip::ObjBranchrule {
public:
	static constexpr int max_priority = 536870911;
	static constexpr int no_maxdepth = -1;
	static constexpr double no_maxbounddist = 1.0;

	LambdaBranchrule(SCIP* scip, const char* name, Func branching_rule);

	auto scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	Func branching_rule;
};

template <typename Func>
scip::LambdaBranchrule<Func>::LambdaBranchrule(SCIP* scip, const char* name, Func branching_rule_) :
	::scip::ObjBranchrule(
		scip,
		"Branchrule that wait for another thread to make the branching.",
		name,
		max_priority,
		no_maxdepth,
		no_maxbounddist),
	branching_rule(std::move(branching_rule_)) {}

template <typename Func>
auto LambdaBranchrule<Func>::scip_execlp(SCIP* scip, SCIP_BRANCHRULE* /*branchrule*/, SCIP_Bool, SCIP_RESULT* result)
	-> SCIP_RETCODE {
	try {
		*result = branching_rule(scip);
		return SCIP_OKAY;
	} catch (...) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_BRANCHERROR;
	}
}

}  // namespace ecole::scip
