#pragma once

#include <exception>
#include <string>

namespace ecole {
namespace scip {

class Exception : public std::exception {
public:
	const std::string message;

	Exception(const std::string& message) : message(message) {}
	const char* what() const noexcept override { return message.c_str(); }
};

} // namespace scip
} // namespace ecole
