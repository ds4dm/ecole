#include <utility>

#include "ecole/exception.hpp"

namespace ecole {

Exception::Exception(std::string message_) noexcept : message(std::move(message_)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace ecole
