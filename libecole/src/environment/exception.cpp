#include <utility>

#include "ecole/environment/exception.hpp"

namespace ecole::environment {

Exception::Exception(std::string message_) : message(std::move(message_)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace ecole::environment
