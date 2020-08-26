#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

class Pseudocosts : public ObservationFunction<nonstd::optional<xt::xtensor<double, 1>>> {
public:
	nonstd::optional<xt::xtensor<double, 1>> obtain_observation(scip::Model& model) override;
};

}  // namespace ecole::observation
