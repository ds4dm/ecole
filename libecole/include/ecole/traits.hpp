#pragma once

#include <tuple>
#include <type_traits>

#include "ecole/utility/function_traits.hpp"

namespace ecole {
namespace trait {

template <typename...> using void_t = void;

/***********************
 *  General detection  *
 ***********************/

template <typename, typename = void_t<>>
struct is_observation_function : std::false_type {};

template <typename T>
struct is_observation_function<T, void_t<decltype(&T::obtain_observation)>> :
	std::true_type {};

template <typename, typename = void_t<>> struct is_environment : std::false_type {};

template <typename T>
struct is_environment<T, void_t<decltype(&T::step)>> : std::true_type {};

template <typename, typename = void_t<>> struct is_dynamics : std::false_type {};

template <typename T>
struct is_dynamics<T, void_t<decltype(&T::step_dynamics)>> : std::true_type {};

/***********************************
 *  Detection of observation type  *
 ***********************************/

template <typename, typename = void> struct observation_of;

template <typename T>
struct observation_of<T, std::enable_if_t<is_observation_function<T>::value>> {
	using type = utility::return_t<decltype(&T::obtain_observation)>;
};

template <typename T>
struct observation_of<T, std::enable_if_t<is_environment<T>::value>> {
	using type = std::tuple_element_t<0, utility::return_t<decltype(&T::step)>>;
};

template <typename T> using observation_of_t = typename observation_of<T>::type;

/******************************
 *  Detection of action type  *
 ******************************/

template <typename, typename = void> struct action_of;

template <typename T> struct action_of<T, std::enable_if_t<is_environment<T>::value>> {
	using type = std::decay_t<utility::arg_t<1, decltype(&T::step)>>;
};

template <typename T> struct action_of<T, std::enable_if_t<is_dynamics<T>::value>> {
	using type = std::decay_t<utility::arg_t<2, decltype(&T::step_dynamics)>>;
};

template <typename T> using action_of_t = typename action_of<T>::type;

/*****************************
 *  Detection of action set  *
 *****************************/

template <typename, typename = void> struct action_set_of;

template <typename T>
struct action_set_of<T, std::enable_if_t<is_environment<T>::value>> {
	using type = std::tuple_element_t<1, utility::return_t<decltype(&T::step)>>;
};

template <typename T> struct action_set_of<T, std::enable_if_t<is_dynamics<T>::value>> {
	using type = std::tuple_element_t<1, utility::return_t<decltype(&T::step_dynamics)>>;
};

template <typename T> using action_set_of_t = typename action_set_of<T>::type;

}  // namespace trait
}  // namespace ecole
