#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

class Pseudocosts : public ObservationFunction<std::optional<xt::xtensor<double, 1>>> {
public:
	std::optional<xt::xtensor<double, 1>> extract(scip::Model& model, bool done) override;
};

}  // namespace ecole::observation
