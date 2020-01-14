#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "xtensor/xtensor.hpp"

#include "ecole/base.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace obs {

struct BasicObs {
	using dtype = double;

	xt::xtensor<dtype, 2, xt::layout_type::row_major> var_feat;
};

struct BasicObsSpace : public base::ObservationSpace<BasicObs> {
	using obs_t = BasicObs;

	std::unique_ptr<ObservationSpace> clone() const override;

	obs_t get(scip::Model const& model) override;
};

}  // namespace obs
}  // namespace ecole
