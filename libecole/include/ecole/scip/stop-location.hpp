#pragma once

#include <scip/type_timing.h>
#include <tuple>
#include <variant>

namespace ecole::scip {

enum struct Callback { Branchrule, Heurisitc };

constexpr auto callback_name(Callback location) {
	switch (location) {
	case Callback::Branchrule:
		return "ecole::scip::StopLocation::Branchrule";
	case Callback::Heurisitc:
		return "ecole::scip::StopLocation::Heurisitc";
	}
}

struct CallbackConstant {
	static constexpr inline int priority_max = 536870911;
	static constexpr inline int maxdepth_none = -1;
	static constexpr inline double maxbounddist_none = 1.0;
	static constexpr inline int frequency_always = 1;
	static constexpr inline int frequency_offset_none = 0;
};

template <Callback callback> struct CallbackConstructorArgs;

template <> struct CallbackConstructorArgs<Callback::Branchrule> {
	int priority = CallbackConstant::priority_max;
	int maxdepth = CallbackConstant::maxdepth_none;
	double maxbounddist = CallbackConstant::maxbounddist_none;
};

template <> struct CallbackConstructorArgs<Callback::Heurisitc> {
	int priority = CallbackConstant::priority_max;
	int frequency = CallbackConstant::frequency_always;
	int frequency_offset = CallbackConstant::frequency_offset_none;
	int maxdepth = CallbackConstant::maxdepth_none;
	SCIP_HEURTIMING timingmask = SCIP_HEURTIMING_AFTERNODE;
};

using DynamicCallbackConstructor =
	std::variant<CallbackConstructorArgs<Callback::Branchrule>, CallbackConstructorArgs<Callback::Heurisitc>>;

}  // namespace ecole::scip
