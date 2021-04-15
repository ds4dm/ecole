#pragma once

#include <cstdint>

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class NNodes : public RewardFunction {
public:
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	std::uint64_t last_n_nodes = 0;
};

}  // namespace ecole::reward
