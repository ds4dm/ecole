#pragma once

#include <tuple>
#include <type_traits>

#include "ecole/information/abstract.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/utility/function-traits.hpp"

namespace ecole::trait {

/*********************************
 *  Detection of data functions  *
 *********************************/

template <typename T> using is_reward = std::is_same<T, reward::Reward>;
template <typename T> inline constexpr bool is_reward_v = is_reward<T>::value;

template <typename T> struct is_information_map : std::false_type {};
template <typename I> struct is_information_map<information::InformationMap<I>> : std::true_type {};
template <typename T> inline constexpr bool is_information_map_v = is_information_map<T>::value;

namespace internal {

template <typename, typename = std::void_t<>> struct has_before_reset : std::false_type {};
template <typename T> struct has_before_reset<T, std::void_t<decltype(&T::before_reset)>> : std::true_type {};

template <typename, typename = std::void_t<>> struct has_extract : std::false_type {};
template <typename T> struct has_extract<T, std::void_t<decltype(&T::extract)>> : std::true_type {};

template <typename, template <typename> typename, typename = std::void_t<>>
struct extract_return_is : std::false_type {};
template <typename T, template <typename> typename Pred>
struct extract_return_is<T, Pred, std::void_t<decltype(&T::extract)>> :
	Pred<utility::return_t<decltype(&T::extract)>> {};

}  // namespace internal

template <typename T>
using is_data_function = std::conjunction<internal::has_before_reset<T>, internal::has_extract<T>>;
template <typename T> inline constexpr bool is_data_function_v = is_data_function<T>::value;

template <typename T> using is_observation_function = is_data_function<T>;
template <typename T> inline constexpr bool is_observation_function_v = is_observation_function<T>::value;

template <typename T>
using is_reward_function = std::conjunction<is_data_function<T>, internal::extract_return_is<T, is_reward>>;
template <typename T> inline constexpr bool is_reward_function_v = is_reward_function<T>::value;

template <typename T>
using is_information_function =
	std::conjunction<is_data_function<T>, internal::extract_return_is<T, is_information_map>>;
template <typename T> inline constexpr bool is_information_function_v = is_information_function<T>::value;

/******************************
 *  Detection of environment  *
 ******************************/

namespace internal {

template <typename, typename = std::void_t<>> struct has_template_step : std::false_type {};
template <typename T> struct has_template_step<T, std::void_t<decltype(&T::template step<>)>> : std::true_type {};
template <typename T> inline constexpr bool has_template_step_v = has_template_step<T>::value;

}  // namespace internal

template <typename T> inline constexpr bool is_environment_v = internal::has_template_step_v<T>;

/***************************
 *  Detection of dynamics  *
 ***************************/

namespace internal {

template <typename, typename = std::void_t<>> struct has_step_dynamics : std::false_type {};
template <typename T> struct has_step_dynamics<T, std::void_t<decltype(&T::step_dynamics)>> : std::true_type {};
template <typename T> inline constexpr bool has_step_dynamics_v = has_step_dynamics<T>::value;

}  // namespace internal

template <typename T> inline constexpr bool is_dynamics_v = internal::has_step_dynamics_v<T>;

/*********************************
 *  Detection of extracted data  *
 *********************************/

template <typename T> using data_of_t = utility::return_t<decltype(&T::extract)>;

/***********************************
 *  Detection of observation type  *
 ***********************************/

template <typename, typename = void> struct observation_of;

template <typename T> struct observation_of<T, std::enable_if_t<is_observation_function_v<T>>> {
	using type = data_of_t<T>;
};

template <typename T> struct observation_of<T, std::enable_if_t<is_environment_v<T>>> {
	using type = std::tuple_element_t<0, utility::return_t<decltype(&T::template step<>)>>;
};

template <typename T> using observation_of_t = typename observation_of<T>::type;

/***********************************
 *  Detection of information type  *
 ***********************************/

template <typename, typename = void> struct information_of;

template <typename T> struct information_of<T, std::enable_if_t<is_information_function_v<T>>> {
	using type = typename data_of_t<T>::mapped_type;
};

template <typename T> struct information_of<T, std::enable_if_t<is_environment_v<T>>> {
	using type = typename std::tuple_element_t<4, utility::return_t<decltype(&T::template step<>)>>::mapped_type;
};

template <typename T> using information_of_t = typename information_of<T>::type;

/******************************
 *  Detection of action type  *
 ******************************/

template <typename, typename = void> struct action_of;

template <typename T> struct action_of<T, std::enable_if_t<is_environment_v<T>>> {
	using type = std::decay_t<utility::arg_t<1, decltype(&T::template step<>)>>;
};

template <typename T> struct action_of<T, std::enable_if_t<is_dynamics_v<T>>> {
	using type = std::decay_t<utility::arg_t<2, decltype(&T::step_dynamics)>>;
};

template <typename T> using action_of_t = typename action_of<T>::type;

/*****************************
 *  Detection of action set  *
 *****************************/

template <typename, typename = void> struct action_set_of;

template <typename T> struct action_set_of<T, std::enable_if_t<is_environment_v<T>>> {
	using type = std::tuple_element_t<1, utility::return_t<decltype(&T::template step<>)>>;
};

template <typename T> struct action_set_of<T, std::enable_if_t<is_dynamics_v<T>>> {
	using type = std::tuple_element_t<1, utility::return_t<decltype(&T::step_dynamics)>>;
};

template <typename T> using action_set_of_t = typename action_set_of<T>::type;

}  // namespace ecole::trait
