#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/node-bipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model creation") {
	auto env = environment::
		Configuring<observation::NodeBipartite, reward::IsDone, termination::WhenSolved>(
			observation::NodeBipartite{}, reward::IsDone{}, termination::WhenSolved{});

	for (auto i = 0L; i < 2; ++i) {
		env.reset(problem_file);
		env.step({{"conflict/lpiterations", 0}});
	}
}
