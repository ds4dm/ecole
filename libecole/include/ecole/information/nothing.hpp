#pragma once

#include <map>
#include <string>

#include "ecole/information/abstract.hpp"
#include "ecole/none.hpp"

namespace ecole::information {

/**
 * Empty information function.
 */
class Nothing {
public:
	auto before_reset(scip::Model& /*model*/) -> void {}

	auto extract(scip::Model& /* model */, bool /* done */) -> InformationMap<NoneType> { return {}; }
};

}  // namespace ecole::information
