#pragma once

#include <exception>
#include <string>

namespace ecole::environment {

class Exception : public std::exception {
public:
	Exception(std::string message);

	[[nodiscard]] char const* what() const noexcept override;

private:
	std::string message;
};

}  // namespace ecole::environment
