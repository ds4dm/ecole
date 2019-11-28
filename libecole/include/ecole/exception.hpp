#pragma once

#include <exception>
#include <string>

namespace ecole {
namespace scip {

class Exception : public std::exception {
public:
	Exception(const std::string& message);
	const char* what() const noexcept override;

private:
	const std::string message;
};

}  // namespace scip

namespace base {

class Exception : public std::exception {
public:
	Exception(const std::string& message);
	const char* what() const noexcept override;

private:
	const std::string message;
};

}  // namespace base
}  // namespace ecole
