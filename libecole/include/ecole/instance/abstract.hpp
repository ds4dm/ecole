#pragma once

#include "ecole/export.hpp"
#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::instance {

/**
 * Base class for instance generators.
 */
class ECOLE_EXPORT InstanceGenerator {
public:
	virtual ~InstanceGenerator() = default;

	/**
	 * Generate new instance by partially consuming the internal random generator.
	 */
	ECOLE_EXPORT virtual scip::Model next() = 0;

	/**
	 * Seed the internal random generator.
	 */
	ECOLE_EXPORT virtual void seed(Seed seed) = 0;

	/**
	 * Wether the generator is exhausted.
	 */
	[[nodiscard]] ECOLE_EXPORT virtual bool done() const = 0;
};

}  // namespace ecole::instance
