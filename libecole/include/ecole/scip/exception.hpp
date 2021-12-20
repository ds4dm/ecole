#pragma once

#include <exception>
#include <string>

#include <scip/scip.h>

#include "ecole/export.hpp"

namespace ecole::scip {

class ECOLE_EXPORT ScipError : public std::exception {
public:
	ECOLE_EXPORT static ScipError from_retcode(SCIP_RETCODE retcode);
	ECOLE_EXPORT static void reset_message_capture();

	ECOLE_EXPORT ScipError(std::string message);

	[[nodiscard]] ECOLE_EXPORT char const* what() const noexcept override;

private:
	std::string message;
};

}  // namespace ecole::scip
