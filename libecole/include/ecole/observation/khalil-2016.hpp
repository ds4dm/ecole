#pragma once

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

using Khalil2016Obs = xt::xtensor<double, 2>;

class Khalil2016 : public ObservationFunction<nonstd::optional<Khalil2016Obs>> {
public:
	nonstd::optional<Khalil2016Obs> obtain_observation(scip::Model& model) override;
};

}  // namespace observation
}  // namespace ecole
