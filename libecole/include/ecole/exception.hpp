#pragma once

#include <stdexcept>
#include <string>

#include "ecole/export.hpp"

namespace ecole {

/*
 * Exception class indicating that the environment interface is not use in the intended way.
 */
class ECOLE_EXPORT MarkovError : public std::logic_error {
public:
	using std::logic_error::logic_error;
};

/*
 * Exception class indicating that an generator cannot generate any new items.
 */
class ECOLE_EXPORT IteratorExhausted : public std::logic_error {
public:
	using std::logic_error::logic_error;

	ECOLE_EXPORT IteratorExhausted();
};

}  // namespace ecole
