#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/none.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class Nothing : public ObservationFunction<NoneType> {
public:
	using Base = ObservationFunction<NoneType>;

	NoneType obtain_observation(environment::State const&) override { return None; }
};

}  // namespace observation
}  // namespace ecole
