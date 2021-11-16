#include <exception>
#include <iostream>
#include <tuple>
#include <utility>

#include "ecole/environment/branching.hpp"
#include "ecole/information/nothing.hpp"
#include "ecole/instance/set-cover.hpp"
#include "ecole/observation/node-bipartite.hpp"
#include "ecole/reward/n-nodes.hpp"

int main() {
	try {
		auto env = ecole::environment::
			Branching<ecole::observation::NodeBipartite, ecole::reward::NNodes, ecole::information::Nothing>{};
		std::size_t constexpr n_rows = 100;
		std::size_t constexpr n_cols = 200;
		auto gen = ecole::instance::SetCoverGenerator{{n_rows, n_cols}};

		static constexpr auto n_episodes = 2;
		for (std::size_t i = 0; i < n_episodes; ++i) {
			auto [obs, action_set, reward, done, info] = env.reset(gen.next());
			while (!done && action_set.has_value()) {
				std::tie(obs, action_set, reward, done, info) = env.step(action_set.value()[0]);
			}
		}
	} catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << '\n';
	}
}
