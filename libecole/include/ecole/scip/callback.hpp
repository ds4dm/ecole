#pragma once

#include <scip/type_timing.h>
#include <tuple>
#include <variant>

/**
 * Reverse callback tools.
 *
 * Helper tools for using reverse callback for iterative solving.
 */
namespace ecole::scip::callback {

/** Type of rverse callback available. */
enum struct Type { Branchrule, Heurisitc };

/** Return the name used for the reverse callback. */
constexpr auto name(Type type) {
	switch (type) {
	case Type::Branchrule:
		return "ecole::scip::StopLocation::Branchrule";
	case Type::Heurisitc:
		return "ecole::scip::StopLocation::Heurisitc";
	}
}

constexpr inline int priority_max = 536870911;
constexpr inline int maxdepth_none = -1;
constexpr inline double maxbounddist_none = 1.0;
constexpr inline int frequency_always = 1;
constexpr inline int frequency_offset_none = 0;

/** Parameter passed to create a reverse callback. */
template <Type type> struct Constructor;

/** Parameter passed to a reverse branchrule. */
template <> struct Constructor<Type::Branchrule> {
	int priority = priority_max;
	int maxdepth = maxdepth_none;
	double maxbounddist = maxbounddist_none;
};
using BranchruleConstructor = Constructor<Type::Branchrule>;

/** Parameter passed to create a reverse heurisitc. */
template <> struct Constructor<Type::Heurisitc> {
	int priority = priority_max;
	int frequency = frequency_always;
	int frequency_offset = frequency_offset_none;
	int maxdepth = maxdepth_none;
	SCIP_HEURTIMING timing_mask = SCIP_HEURTIMING_AFTERNODE;
};
using HeuristicConstructor = Constructor<Type::Heurisitc>;

using DynamicConstructor = std::variant<Constructor<Type::Branchrule>, Constructor<Type::Heurisitc>>;

}  // namespace ecole::scip::callback
