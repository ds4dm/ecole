#pragma once

#include <memory>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace termination {

struct Solved : public base::TerminationSpace {
	std::unique_ptr<base::TerminationSpace> clone() const override;
	bool is_done(scip::Model const& model) override;
};

}  // namespace termination
}  // namespace ecole
