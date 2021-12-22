#pragma once

#include "ecole/none.hpp"

namespace ecole::scip {
class Model;
}

namespace ecole::data {

class NoneFunction {
public:
	auto before_reset(scip::Model const& /*model*/) -> void {}

	auto extract(scip::Model const& /*model*/, bool /*done*/) -> NoneType { return ecole::None; }
};

}  // namespace ecole::data
