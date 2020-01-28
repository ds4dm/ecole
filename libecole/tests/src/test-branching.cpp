#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/branching.hpp"
#include "ecole/exception.hpp"
#include "ecole/observation/basicobs.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

scip::Model get_model() {
	auto model = scip::Model::from_file(problem_file);
	model.disable_cuts();
	model.disable_presolve();
	return model;
}

TEST_CASE("BranchEnv") {
	using BranchEnv = branching::Environment<std::size_t, observation::BasicObs<>>;
	auto env = BranchEnv(
		std::make_unique<branching::Fractional>(),
		std::make_unique<observation::BasicObsFunction<>>(),
		std::make_unique<reward::IsDone>(),
		std::make_unique<termination::WhenSolved>());
	auto model = get_model();

	SECTION("reset, reset, and delete") {
		env.reset(scip::Model{model});
		env.reset(std::move(model));
	}

	SECTION("reset, step, and delete") {
		env.reset(std::move(model));
		env.step(0);
	}

	SECTION("run full trajectory") {
		auto run_trajectory = [](auto& env, auto& model) {
			auto obs_done = env.reset(scip::Model{model});
			auto done = std::get<bool>(obs_done);
			auto count = 0;
			while (!done) {
				auto obs_rew_done_info = env.step(0);
				done = std::get<bool>(obs_rew_done_info);
				++count;
			}
			REQUIRE(count > 0);
		};
		run_trajectory(env, model);

		SECTION("Run another trajectory") { run_trajectory(env, model); }
	}

	SECTION("manage errors") {
		auto guard = ScipNoErrorGuard{};
		env.reset(std::move(model));
		REQUIRE_THROWS_AS(env.step(-1), scip::Exception);
	}
}
