#pragma once

#include <memory>
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

	BasicObs get(scip::Model const& model) const override;
	std::unique_ptr<ObservationSpace> clone() const override;
};

} // namespace obs
} // namespace ecole
