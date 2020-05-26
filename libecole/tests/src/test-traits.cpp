#include <type_traits>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/traits.hpp"

using namespace ecole;

#define STATIC_ASSERT(expr) static_assert(expr, #expr);
#define STATIC_ASSERT_NOT(expr) static_assert(!(expr), #expr);
#define STATIC_ASSERT_SAME(A, B) \
	static_assert(std::is_same<A, B>::value, "Types are the same")
#define STATIC_ASSERT_DIFFERENT(A, B) \
	static_assert(!std::is_same<A, B>::value, "Types are different")

/**********************************
 *  Test is_observation_function  *
 **********************************/

// Positive tests
STATIC_ASSERT(trait::is_observation_function<observation::Nothing>::value);
// Negative tests
STATIC_ASSERT_NOT(trait::is_observation_function<ecole::NoneType>::value);
STATIC_ASSERT_NOT(trait::is_observation_function<environment::Configuring<>>::value);

/*************************
 *  Test is_environment  *
 *************************/

// Positive tests
STATIC_ASSERT(trait::is_environment<environment::Configuring<>>::value);
// Negative tests
STATIC_ASSERT_NOT(trait::is_environment<environment::ConfiguringDynamics>::value);
STATIC_ASSERT_NOT(trait::is_environment<observation::Nothing>::value);

/*************************
 *  Test observation_of  *
 *************************/

// Positive tests
STATIC_ASSERT_SAME(trait::observation_of_t<observation::Nothing>, ecole::NoneType);
// Negative tests
STATIC_ASSERT_DIFFERENT(trait::observation_of_t<observation::Nothing>, int);
