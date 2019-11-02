#pragma once

#include <vector>

#include "ecole/env/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace env {

struct BasicObs {
	std::vector<double> ubs;
};

struct BasicObsSpace : public ObservationSpace<BasicObs> {
	BasicObs get(scip::Model const& model);
};

} // namespace env
} // namespace ecole
