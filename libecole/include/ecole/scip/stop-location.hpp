#pragma once

#include <array>

namespace ecole::scip {

enum struct StopLocation { Branchrule, Heurisitc };

constexpr auto callback_name(StopLocation location) {
	switch (location) {
	case StopLocation::Branchrule:
		return "ecole::scip::StopLocation::Branchrule";
	case StopLocation::Heurisitc:
		return "ecole::scip::StopLocation::Heurisitc";
	}
}

}  // namespace ecole::scip
