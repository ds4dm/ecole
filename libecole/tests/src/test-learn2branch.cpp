#include <catch2/catch.hpp>

#include "ecole/learn2branch.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Learn2branch environement can be created from a file") {
	auto env = BranchEnv::from_file(problem_file);
	env.model.disable_cuts();
	env.model.disable_presolve();
	auto count = 0L;
	env.run([&count]() mutable {
		count++;
		return 0L;
	});
	REQUIRE(count > 0);
}
