#pragma once

#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

namespace ecole::python {

template <typename Class, typename... ClassArgs> struct auto_class : public pybind11::class_<Class, ClassArgs...> {
	using pybind11::class_<Class, ClassArgs...>::class_;

	/** An Alternative pybind11::class_::def_readwrite for xtensor members. */
	template <typename Str, typename MemberPtr, typename... Args>
	auto def_readwrite_xtensor(Str&& name, MemberPtr&& member_ptr, Args&&... args) -> auto& {
		using Member = std::remove_reference_t<std::invoke_result_t<MemberPtr, Class>>;
		using value_type = typename Member::value_type;
		auto constexpr rank = xt::get_rank<Member>::value;
		this->def_property(
			std::forward<Str>(name),
			[member_ptr](Class& self) -> decltype(auto) { return std::invoke(member_ptr, self); },
			[member_ptr](Class& self, xt::pytensor<value_type, rank> const& val) { std::invoke(member_ptr, self) = val; },
			std::forward<Args>(args)...);
		return *this;
	}

	/** Copy and deep copy from copy constructor. */
	auto def_auto_copy() -> auto& {
		this->def("__copy__", [](Class const& self) { return std::make_unique<Class>(self); });
		this->def(
			"__deepcopy__",
			[](Class const& self, pybind11::dict const& /*memo*/) { return std::make_unique<Class>(self); },
			pybind11::arg("memo"));
		return *this;
	}

	/** Pickle capbilities using Python attributes.
	 *
	 * The given attributes name must be sufficient to define the object.
	 * They must be bound to Python with read-write capabilities.
	 */
	template <typename... Str> auto def_auto_pickle(Str... names) -> auto& {
		this->def(pybind11::pickle(
			[names = std::array{names...}](pybind11::handle self) {
				auto dict = pybind11::dict{};
				for (auto const& name : names) {
					dict[name] = self.attr(name);
				}
				return dict;
			},
			[names = std::array{names...}](pybind11::dict const& dict) {
				// Constructor may not be bound so we create the object from C++ and cast it
				auto obj = std::make_unique<Class>();
				auto py_obj = pybind11::cast(obj.get());
				for (auto const& name : names) {
					py_obj.attr(name) = dict[name];
				}
				return obj;
			}));
		return *this;
	}
};

/** Hold a class member variable function pointer and its name together. */
template <typename FuncPtr> struct Member {
	char const* name;
	FuncPtr value;

	constexpr Member(char const* the_name, FuncPtr the_value) : name{the_name}, value{the_value} {}
};

/** Utility to bind a data class, that is C-struct, named-tuple... like class. */
template <typename Class, typename... ClassArgs> struct auto_data_class : public auto_class<Class, ClassArgs...> {
	using auto_class<Class, ClassArgs...>::auto_class;
	using typename pybind11::class_<Class, ClassArgs...>::type;

	template <typename... FuncPtr> auto def_auto_members(Member<FuncPtr>... members) -> auto& {
		def_auto_init(members...);
		def_auto_attributes(members...);
		this->def_auto_copy();
		this->def_auto_pickle(members.name...);
		return *this;
	}

	template <typename... FuncPtr> auto def_auto_init(Member<FuncPtr>... members) -> auto& {
		// Instantiate the C++ type at compile time to get default parameters.
		auto constexpr default_params = type{};
		// Bind a constructor that takes as input all parameters
		this->def(
			// Get the type of each parameter and add it to the Python constructor
			pybind11::init<std::remove_reference_t<std::invoke_result_t<decltype(members.value), type>>...>(),
			// Set name for all constructor parameters and fetch default value on the default parameters
			(pybind11::arg(members.name) = std::invoke(members.value, default_params))...);
		return *this;
	}

	/** Bind attribute access for all class attributes. */
	template <typename... FuncPtr> auto def_auto_attributes(Member<FuncPtr>... members) -> auto& {
		((this->def_readwrite(members.name, members.value)), ...);
		return *this;
	}
};

}  // namespace ecole::python
