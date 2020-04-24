#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class NothingObs {};

class Nothing : public ObservationFunction<NothingObs> {
public:
	using Base = ObservationFunction<NothingObs>;

	NothingObs obtain_observation(environment::State const&) override { return {}; }
};

}  // namespace observation
}  // namespace ecole
