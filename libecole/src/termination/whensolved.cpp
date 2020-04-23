#include "ecole/termination/whensolved.hpp"

namespace ecole {
namespace termination {

bool WhenSolved::is_done(environment::State const& state) {
	return state.model.is_solved();
}

}  // namespace termination
}  // namespace ecole
