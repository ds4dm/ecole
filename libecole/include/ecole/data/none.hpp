#pragma once

#include "ecole/data/abstract.hpp"
#include "ecole/none.hpp"

namespace ecole::data {

class NoneFunction : public DataFunction<NoneType> {
public:
	NoneType extract(scip::Model& /* model */, bool /* done */) override { return ecole::None; }
};

}  // namespace ecole::data
