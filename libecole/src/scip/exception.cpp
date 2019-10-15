#include "ecole/scip/exception.hpp"

namespace ecole {
namespace scip {

Exception::Exception(const std::string& message) : message(message) {}

const char* Exception::what() const noexcept { return message.c_str(); }

} // namespace scip
} // namespace ecole
