#include "ecole/exception.hpp"

namespace ecole {
namespace scip {

Exception::Exception(const std::string& message) : message(message) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace scip

namespace base {

Exception::Exception(const std::string& message) : message(message) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace base

}  // namespace ecole
