#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/learn2branch.hpp"
#include "ecole/scip/exception.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("BranchEnv can be created from a file") {
	auto env = BranchEnv::from_file(problem_file);
}

TEST_CASE("BranchEnv can run a branching function") {
	auto env = BranchEnv::from_file(problem_file);
	env.model.disable_cuts();
	env.model.disable_presolve();

	SECTION("run a branching function") {
		auto count_branch = [](BranchEnv& env) {
			auto count = 0L;
			env.run([&count](auto obs) mutable {
				count++;
				return 0L;
			});
			return count;
		};
		REQUIRE(count_branch(env) > 0);

		SECTION("run two branching functions") { REQUIRE(count_branch(env) > 0); }
	}

	SECTION("manage errors") {
		auto guard = ScipNoErrorGuard{};
		auto bad_func = [](auto obs) {
			throw std::runtime_error("BadError");
			return 0L;
		};
		REQUIRE_THROWS_AS(env.run(bad_func), scip::Exception);
	}
}
