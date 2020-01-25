#pragma once

#include <memory>

#include "ecole/base.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

struct Done : public base::RewardFunction {
	std::unique_ptr<RewardFunction> clone() const override;
	reward_t get(scip::Model const& model, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
