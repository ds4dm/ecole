#include "ecole/termination/whensolved.hpp"

namespace ecole {
namespace termination {

auto WhenSolved::clone() const -> std::unique_ptr<TerminationFunction> {
	return std::make_unique<WhenSolved>(*this);
}

bool WhenSolved::is_done(environment::State const& state) {
	return state.model.is_solved();
}

}  // namespace termination
}  // namespace ecole
