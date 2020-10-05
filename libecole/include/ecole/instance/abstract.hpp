#pragma once

#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::instance {

/**
 * Base class for instance generators.
 */
class InstanceGenerator {
public:
	virtual ~InstanceGenerator() = default;

	/**
	 * Generate new instance by partially consuming the internal random engine.
	 */
	virtual scip::Model next() = 0;

	/**
	 * Seed the internal random engine.
	 */
	virtual void seed(Seed seed) = 0;
};

}  // namespace ecole::instance
