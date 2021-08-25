#pragma once

#include <exception>
#include <string>

#include "ecole/export.hpp"

namespace ecole {

class ECOLE_EXPORT Exception : public std::exception {
public:
	ECOLE_EXPORT Exception(std::string message) noexcept;

	[[nodiscard]] ECOLE_EXPORT char const* what() const noexcept override;

private:
	std::string message;
};

class ECOLE_EXPORT IteratorExhausted : public Exception {
public:
	using Exception::Exception;

	ECOLE_EXPORT IteratorExhausted();
};

}  // namespace ecole
