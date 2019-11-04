#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/env/learn2branch.hpp"
#include "ecole/exception.hpp"

#include "conftest.hpp"

using namespace ecole;

// scip::Model get_model() {
//   auto model = scip::Model::from_file(problem_file);
//   model.disable_cuts();
//   model.disable_presolve();
//   return model;
// }
//
// TEST_CASE("BranchEnv can run a branching function") {
//   auto env = env::BranchEnv{get_model(), std::make_unique<env::BasicObs::Factory>()};
//
//   SECTION("run a branching function") {
//     auto count_branch = [](env::BranchEnv& env) {
//       auto count = 0L;
//       env.run([&count](auto obs) mutable {
//         count++;
//         return 0L;
//       });
//       return count;
//     };
//     REQUIRE(count_branch(env) > 0);
//
//     SECTION("run two branching functions") { REQUIRE(count_branch(env) > 0); }
//   }
//
//   SECTION("manage errors") {
//     auto guard = ScipNoErrorGuard{};
//     auto bad_func = [](auto obs) {
//       throw std::runtime_error("BadError");
//       return 0L;
//     };
//     REQUIRE_THROWS_AS(env.run(bad_func), scip::Exception);
//   }
// }
