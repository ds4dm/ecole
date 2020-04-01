#include "ecole/scip/exception.hpp"

namespace ecole {
namespace scip {

Exception::Exception(std::string const& message_) : message(message_) {}

Exception::Exception(std::string&& message_) : message(std::move(message_)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace scip
}  // namespace ecole
