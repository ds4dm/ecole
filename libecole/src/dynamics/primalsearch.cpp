#include <fmt/format.h>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/primalsearch.hpp"
#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

PrimalSearchDynamics::PrimalSearchDynamics(int trials_per_node_, int depth_freq_, int depth_start_, int depth_stop_) :
	trials_per_node(trials_per_node_), depth_freq(depth_freq_), depth_start(depth_start_), depth_stop(depth_stop_) {
	if (trials_per_node < -1) {
		throw std::invalid_argument{fmt::format("Illegal value for number of trials per node: {}.", trials_per_node)};
	}
}

namespace {

std::optional<VarIds> action_set(scip::Model const& model) {
	if (model.stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto vars = model.pseudo_branch_cands();  // non-fixed discrete variables
	auto var_ids = xt::xtensor<std::size_t, 1>::from_shape({vars.size()});
	std::transform(vars.begin(), vars.end(), var_ids.begin(), SCIPvarGetProbindex);

	return var_ids;
}

}  // namespace

auto PrimalSearchDynamics::reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> {
	heur = model.solve_iter_start_primalsearch(trials_per_node, depth_freq, depth_start, depth_stop);
	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model)};
}

auto PrimalSearchDynamics::step_dynamics(scip::Model& model, VarIdsVals const& action) -> std::tuple<bool, ActionSet> {
	auto const [var_indices, vals] = action;
	auto problem_vars = model.variables();

	// check that both spans have same size
	if (var_indices.size() != vals.size()) {
		throw std::invalid_argument{
			fmt::format("Invalid action: {} variable indices for {} values.", var_indices.size(), vals.size())};
	}

	// check that variable indices are within range
	for (auto const var_id : var_indices) {
		if (var_id >= problem_vars.size()) {
			throw std::invalid_argument{fmt::format("Invalid action: variable index {} is out of range.", var_id)};
		}
	}

	auto* scip_ptr = model.get_scip_ptr();
	SCIP_Bool success = false;  // result of the current action (solution found or not)

	// if the action is not empty, run a search iteration
	// try to improve the (partial) solution by fixing variables and then re-solving the LP
	if (not var_indices.empty()) {
		SCIP_Bool lperror = false;
		SCIP_Bool cutoff = false;

		// enter probing mode
		scip::call(SCIPstartProbing, scip_ptr);

		// fix variables in the (partial) solution to their given values
		for (std::size_t i = 0; i < var_indices.size(); i++) {
			auto id = var_indices[i];
			auto* var = problem_vars[id];
			auto val = vals[i];
			scip::call(SCIPfixVarProbing, scip_ptr, var, val);
		}

		// propagate
		scip::call(SCIPpropagateProbing, scip_ptr, 0, &cutoff, nullptr);
		if (!cutoff) {
			// build the LP if needed
			if (!SCIPisLPConstructed(scip_ptr)) {
				scip::call(SCIPconstructLP, scip_ptr, &cutoff);
			}
			if (!cutoff) {
				// solve the LP
				scip::call(SCIPsolveProbingLP, scip_ptr, -1, &lperror, &cutoff);
				if (!lperror && !cutoff) {
					// try the LP solution in the original problem
					SCIP_SOL* sol = nullptr;
					scip::call(SCIPcreateSol, scip_ptr, &sol, heur);
					scip::call(SCIPlinkLPSol, scip_ptr, sol);
					scip::call(SCIPtrySolFree, scip_ptr, &sol, false, true, true, true, true, &success);
				}
			}
		}

		// exit probing mode
		scip::call(SCIPendProbing, scip_ptr);
	}

	// update the final search result depending on the action result
	if (success) {
		result = SCIP_FOUNDSOL;
	} else if (result == SCIP_DIDNOTRUN) {
		result = SCIP_DIDNOTFIND;
	}

	// increment the number of search trials spent
	trials_spent++;

	// if all trials are exhausted, or if SCIP should be stopped, stop the search and let SCIP proceed
	if ((trials_spent == static_cast<unsigned int>(trials_per_node)) || SCIPisStopped(scip_ptr)) {
		model.solve_iter_primalsearch(result);

		// reset data for the next time the search is triggered
		trials_spent = 0;
		result = SCIP_DIDNOTRUN;
	}

	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model)};
}

}  // namespace ecole::dynamics
