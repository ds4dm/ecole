#include <utility>

#include "ecole/exception.hpp"

namespace ecole {

IteratorExhausted::IteratorExhausted() : std::logic_error{"No item to iterate over."} {}

}  // namespace ecole
