#pragma once

#include <memory>

#include "ecole/none.hpp"
#include "ecole/observation/abstract.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

class Nothing : public ObservationFunction<NoneType> {
public:
	using Base = ObservationFunction<NoneType>;

	NoneType obtain_observation(scip::Model& /* model */, bool /* done */) override { return None; }
};

}  // namespace ecole::observation
