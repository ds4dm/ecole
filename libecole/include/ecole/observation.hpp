#pragma once

#include <vector>

#include "ecole/base/environment.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace obs {

struct BasicObs {
	std::vector<double> ubs;
};

struct BasicObsSpace : public base::ObservationSpace<BasicObs> {
	using obs_t = BasicObs;

	BasicObs get(scip::Model const& model);
};

} // namespace obs
} // namespace ecole
