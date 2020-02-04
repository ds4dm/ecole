#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/configuring.hpp"
#include "ecole/observation/node-bipartite.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model creation") {
	auto env = configuring::Environment<int, observation::NodeBipartiteObs<>>(
		std::make_unique<observation::NodeBipartite<>>(),
		std::make_unique<configuring::Configure<int>>("conflict/lpiterations"));

	for (auto i = 0L; i < 2; ++i) {
		env.reset(get_model());
		env.step(0);
	}
}
