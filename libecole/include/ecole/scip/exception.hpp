#pragma once

#include <exception>
#include <string>

#include <scip/scip.h>

namespace ecole {
namespace scip {

class Exception : public std::exception {
public:
	static Exception from_retcode(SCIP_RETCODE retcode);
	static void reset_message_capture();

	Exception(std::string const& message);
	Exception(std::string&& message);

	char const* what() const noexcept override;

private:
	std::string message;
};

}  // namespace scip
}  // namespace ecole
