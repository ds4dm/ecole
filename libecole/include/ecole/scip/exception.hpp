#pragma once

#include <exception>
#include <string>

namespace ecole {
namespace scip {

class Exception : public std::exception {
public:
	Exception(std::string const& message);
	Exception(std::string&& message);

	char const* what() const noexcept override;

private:
	std::string message;
};

}  // namespace scip
}  // namespace ecole
