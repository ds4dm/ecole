#pragma once

#include <memory>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

struct Done : public base::RewardSpace {
	std::unique_ptr<RewardSpace> clone() const override;
	reward_t get(scip::Model const& model, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
