#include "ecole/scip/exception.hpp"

namespace ecole {
namespace scip {

Exception::Exception(std::string const& message) : message(message) {}

Exception::Exception(std::string&& message) : message(std::move(message)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

}  // namespace scip
}  // namespace ecole
