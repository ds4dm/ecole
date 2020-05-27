#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/scimpl.hpp"

using namespace ecole;

TEST_CASE("Allocation of ressources") {
	scip::Scimpl scimpl{};
	REQUIRE(SCIPgetStage(scimpl.get_scip_ptr()) == SCIP_STAGE_INIT);
}

TEST_CASE("Dealocation of ressources") {
	{ scip::Scimpl scimpl{}; }
	BMScheckEmptyMemory();
}
