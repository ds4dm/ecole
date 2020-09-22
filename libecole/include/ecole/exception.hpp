#pragma once

#include <exception>
#include <string>

namespace ecole {

class Exception : public std::exception {
public:
	Exception(std::string message) noexcept;

	[[nodiscard]] char const* what() const noexcept override;

private:
	std::string message;
};

}  // namespace ecole
