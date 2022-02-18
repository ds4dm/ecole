#pragma once

#include <tuple>
#include <variant>

#include <scip/type_timing.h>

#include "ecole/utility/unreachable.hpp"

/**
 * Reverse callback tools.
 *
 * Helper tools for using reverse callback for iterative solving.
 */
namespace ecole::scip::callback {

/** Type of rverse callback available. */
enum struct Type { Branchrule, Heuristic };

/** Return the name used for the reverse callback. */
constexpr auto name(Type type) {
	switch (type) {
	case Type::Branchrule:
		return "ecole::scip::StopLocation::Branchrule";
	case Type::Heuristic:
		return "ecole::scip::StopLocation::Heuristic";
	default:
		utility::unreachable();
	}
}

constexpr inline int priority_max = 536870911;
constexpr inline int max_depth_none = -1;
constexpr inline double max_bound_distance_none = 1.0;
constexpr inline int frequency_always = 1;
constexpr inline int frequency_offset_none = 0;

/** Parameter passed to create a reverse callback. */
template <Type type> struct Constructor;

/** Parameter passed to a reverse branchrule. */
template <> struct Constructor<Type::Branchrule> {
	int priority = priority_max;
	int max_depth = max_depth_none;
	double max_bound_distance = max_bound_distance_none;
};
using BranchruleConstructor = Constructor<Type::Branchrule>;

/** Parameter passed to create a reverse heurisitc. */
template <> struct Constructor<Type::Heuristic> {
	int priority = priority_max;
	int frequency = frequency_always;
	int frequency_offset = frequency_offset_none;
	int max_depth = max_depth_none;
	SCIP_HEURTIMING timing_mask = SCIP_HEURTIMING_AFTERNODE;
};
using HeuristicConstructor = Constructor<Type::Heuristic>;

using DynamicConstructor = std::variant<Constructor<Type::Branchrule>, Constructor<Type::Heuristic>>;

/** Parameter given by SCIP to the callback function. */
template <Type type> struct Call;

/** Parameter given by SCIP to the branchrule function. */
template <> struct Call<Type::Branchrule> {
	/** The method of the Branchrule callback being called. */
	enum struct Where { LP, External, Pseudo };

	bool allow_add_constraints;
	Where where;
};
using BranchruleCall = Call<Type::Branchrule>;

/** Parameter given by SCIP to the heuristic functions. */
template <> struct Call<Type::Heuristic> {
	SCIP_HEURTIMING heuristic_timing;
	bool node_infeasible;
};
using HeuristicCall = Call<Type::Heuristic>;

using DynamicCall = std::variant<Call<Type::Branchrule>, Call<Type::Heuristic>>;

}  // namespace ecole::scip::callback
