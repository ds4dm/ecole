#include "ecole/environment/exception.hpp"

namespace ecole {
namespace environment {

Exception::Exception(std::string const& message) : message(message) {}

Exception::Exception(std::string&& message) : message(std::move(message)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace environment
}  // namespace ecole
