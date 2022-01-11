#pragma once

#include "ecole/data/abstract.hpp"
#include "ecole/none.hpp"

namespace ecole::data {

class NoneFunction {
public:
	auto before_reset(scip::Model const& /*model*/) -> void {}

	auto extract(scip::Model const& /*model*/, bool /*done*/) -> NoneType { return ecole::None; }
};

}  // namespace ecole::data
