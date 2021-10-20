#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/export.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

class ECOLE_EXPORT Pseudocosts : public ObservationFunction<std::optional<xt::xtensor<double, 1>>> {
public:
	ECOLE_EXPORT auto extract(scip::Model& model, bool done) -> std::optional<xt::xtensor<double, 1>> override;
};

}  // namespace ecole::observation
