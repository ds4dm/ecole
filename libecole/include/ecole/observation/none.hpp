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

	std::unique_ptr<Base> clone() const override { return std::make_unique<None>(*this); }

	NoneObs get(environment::State const&) override { return {}; }
};

}  // namespace observation
}  // namespace ecole
