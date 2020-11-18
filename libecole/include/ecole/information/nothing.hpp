#pragma once

#include <map>
#include <string>

#include "ecole/information/abstract.hpp"
#include "ecole/none.hpp"

namespace ecole::information {

/**
 * Empty information function.
 */
class Nothing : public InformationFunction<NoneType> {
public:
	std::map<std::string, NoneType> extract(scip::Model& /* model */, bool /* done */) override { return {}; }
};

}  // namespace ecole::information
