#pragma once

#include <functional>
#include <memory>
#include <type_traits>

#include <xtensor-python/pytensor.hpp>

#include <pybind11/pybind11.h>

namespace ecole {

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
	template <typename StrArr> auto def_auto_pickle(StrArr const& names) -> auto& {
		this->def(pybind11::pickle(
			[names](pybind11::handle self) {
				auto dict = pybind11::dict{};
				for (auto const& name : names) {
					dict[name] = self.attr(name);
				}
				return dict;
			},
			[names](pybind11::dict const& dict) {
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

}  // namespace ecole
