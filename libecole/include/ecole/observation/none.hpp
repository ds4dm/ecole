#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class NoneObs {};

class None : public ObservationFunction<NoneObs> {
public:
	using Base = ObservationFunction<NoneObs>;

	NoneObs get(environment::State const&) override { return {}; }
};

}  // namespace observation
}  // namespace ecole
