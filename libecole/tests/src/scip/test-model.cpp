#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

class ScipNoErrorGuard {
public:
	ScipNoErrorGuard() { SCIPmessageSetErrorPrinting(nullptr, nullptr); }
	~ScipNoErrorGuard() { SCIPmessageSetErrorPrintingDefault(); }
};

TEST_CASE("Allocation of ressources") {
	auto&& scip = scip::create();
	REQUIRE(SCIPgetStage(scip.get()) == SCIP_STAGE_INIT);
}

TEST_CASE("Dealocation of ressources") {
	auto scip = scip::create();
	scip.reset();
	REQUIRE(scip == nullptr);
	REQUIRE(BMSgetMemoryUsed() == 0);
}

TEST_CASE("Creation of model") {
	auto model = scip::Model{};
	SECTION("Copy construct") { auto model_copy = model; }
	SECTION("Move construct") { auto model_moved = std::move(model); }
}

TEST_CASE("Creation of model from scip pointer") {
	REQUIRE_THROWS_AS(scip::Model(nullptr), scip::ScipException);
	scip::Model{scip::create()};
}

TEST_CASE("Create model from file") { auto model = scip::Model::from_file(problem_file); }

TEST_CASE("Raise if file does not exist") {
	auto guard = ScipNoErrorGuard{};
	REQUIRE_THROWS_AS(scip::Model::from_file("/does_not_exist.mps"), scip::ScipException);
}

TEST_CASE("Solve a model") {
	auto model = scip::Model::from_file(problem_file);
	model.solve();
}

TEST_CASE("Add a branching rule") {
	auto model = scip::Model::from_file(problem_file);
	model.disable_presolve();
	model.disable_cuts();
	auto count = 0;
	model.set_branch_rule([&count](scip::Model const& m) mutable {
		count++;
		return *m.lp_branch_vars().cbegin();
	});
	model.solve();
	REQUIRE(count > 0);
}
