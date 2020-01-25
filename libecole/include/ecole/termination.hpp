#pragma once

#include <memory>

#include "ecole/base.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace termination {

struct Solved : public base::TerminationFunction {
	std::unique_ptr<base::TerminationFunction> clone() const override;
	bool is_done(scip::Model const& model) override;
};

}  // namespace termination
}  // namespace ecole
