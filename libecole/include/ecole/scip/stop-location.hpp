#pragma once

#include <scip/type_timing.h>
#include <tuple>

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

template <> struct CallbackConstructorArgs<Callback::Branchrule> : CallbackConstant {
	int priority = priority_max;
	int maxdepth = maxdepth_none;
	double maxbounddist = maxbounddist_none;
};

template <> struct CallbackConstructorArgs<Callback::Heurisitc> : CallbackConstant {
	int priority = priority_max;
	int frequency = frequency_always;
	int frequency_offset = frequency_offset_none;
	int maxdepth = maxdepth_none;
	SCIP_HEURTIMING timingmask = SCIP_HEURTIMING_AFTERNODE;
};

}  // namespace ecole::scip
