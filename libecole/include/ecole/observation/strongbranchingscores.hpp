#pragma once

#include <memory>
#include <optional>

#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/export.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

class ECOLE_EXPORT StrongBranchingScores : public ObservationFunction<std::optional<xt::xtensor<double, 1>>> {
public:
	ECOLE_EXPORT StrongBranchingScores(bool pseudo_candidates = true);

	ECOLE_EXPORT std::optional<xt::xtensor<double, 1>> extract(scip::Model& model, bool done) override;

private:
	bool pseudo_candidates;
};

}  // namespace ecole::observation
