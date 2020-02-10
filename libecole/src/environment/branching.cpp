
#include <objscip/objbranchrule.h>

#include "ecole/environment/branching.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace environment {

namespace {

class ReverseBranchrule : public ::scip::ObjBranchrule {
public:
	auto scip_init(SCIP* scip, SCIP_BRANCHRULE* branchrule) -> SCIP_RETCODE override;

	auto scip_execlp(
		SCIP* scip,
		SCIP_BRANCHRULE* branchrule,
		SCIP_Bool allowaddcons,
		SCIP_RESULT* result) -> SCIP_RETCODE override;
};

}  // namespace

}  // namespace environment
}  // namespace ecole
