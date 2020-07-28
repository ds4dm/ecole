#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class Pseudocosts : public ObservationFunction<nonstd::optional<xt::xtensor<double, 1>>> {
public:
	nonstd::optional<xt::xtensor<double, 1>> obtain_observation(scip::Model& state) override;
};

}  // namespace observation
}  // namespace ecole
