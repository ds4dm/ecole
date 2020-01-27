#pragma once

#include <memory>

#include "ecole/scip/model.hpp"
#include "ecole/termination/base.hpp"

namespace ecole {
namespace termination {

class WhenSolved : public TerminationFunction {
public:
	std::unique_ptr<TerminationFunction> clone() const override;
	bool is_done(scip::Model const& model) override;
};

}  // namespace termination
}  // namespace ecole
