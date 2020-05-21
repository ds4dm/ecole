#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/scipimpl.hpp"

using namespace ecole;

TEST_CASE("Allocation of ressources") {
	scip::ScipImpl scipimpl{};
	REQUIRE(SCIPgetStage(scipimpl.get_scip_ptr()) == SCIP_STAGE_INIT);
}

TEST_CASE("Dealocation of ressources") {
	{ scip::ScipImpl scipimpl{}; }
	BMScheckEmptyMemory();
}
