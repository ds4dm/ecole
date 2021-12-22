#pragma once

#include <memory>
#include <optional>

#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/export.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

class ECOLE_EXPORT StrongBranchingScores {
public:
	ECOLE_EXPORT StrongBranchingScores(bool pseudo_candidates = false);

	auto before_reset(scip::Model& /*model*/) -> void {}

	ECOLE_EXPORT auto extract(scip::Model& model, bool done) -> std::optional<xt::xtensor<double, 1>>;

private:
	bool pseudo_candidates;
};

}  // namespace ecole::observation
