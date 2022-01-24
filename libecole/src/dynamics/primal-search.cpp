#include <algorithm>
#include <exception>
#include <stdexcept>

#include <fmt/format.h>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/primal-search.hpp"
#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/stop-location.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

PrimalSearchDynamics::PrimalSearchDynamics(int trials_per_node_, int depth_freq_, int depth_start_, int depth_stop_) :
	trials_per_node(trials_per_node_), depth_freq(depth_freq_), depth_start(depth_start_), depth_stop(depth_stop_) {
	if (trials_per_node < -1) {
		throw std::invalid_argument{fmt::format("Illegal value for number of trials per node: {}.", trials_per_node)};
	}
}

namespace {

auto action_set(scip::Model const& model) -> PrimalSearchDynamics::ActionSet {
	if (model.stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto vars = model.pseudo_branch_cands();  // non-fixed discrete variables
	auto var_ids = xt::xtensor<std::size_t, 1>::from_shape({vars.size()});
	std::transform(vars.begin(), vars.end(), var_ids.begin(), SCIPvarGetProbindex);

	return var_ids;
}

auto add_solution_from_lp(SCIP* scip) -> bool {
	SCIP_Bool solution_kept = false;
	SCIP_SOL* sol = nullptr;
	auto* const heur = SCIPfindHeur(scip, scip::callback_name(scip::Callback::Heurisitc));
	scip::call(SCIPcreateSol, scip, &sol, heur);
	try {
		scip::call(SCIPlinkLPSol, scip, sol);
	} catch (std::exception const& e) {
		// In case of failure, the solution must be free anyway.
		scip::call(SCIPtrySolFree, scip, &sol, false, true, true, true, true, &solution_kept);
		throw;
	}
	scip::call(SCIPtrySolFree, scip, &sol, false, true, true, true, true, &solution_kept);
	return solution_kept;
}

}  // namespace

auto PrimalSearchDynamics::reset_dynamics(scip::Model& model) const -> std::tuple<bool, ActionSet> {
	if (trials_per_node == 0) {
		model.solve();
		return {true, {}};
	}
	auto const args = scip::CallbackConstructorArgs<scip::Callback::Heurisitc>{
		scip::CallbackConstant::priority_max, depth_freq, depth_start, depth_stop};
	if (model.solve_iter(args).has_value()) {
		return {false, action_set(model)};
	}
	return {true, {}};
}

auto PrimalSearchDynamics::step_dynamics(scip::Model& model, Action action) -> std::tuple<bool, ActionSet> {
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
	auto solution_kept = false;  // result of the current action (solution found or not)

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
					solution_kept = add_solution_from_lp(scip_ptr);
				}
			}
		}

		// exit probing mode
		scip::call(SCIPendProbing, scip_ptr);
	}

	// update the final search result depending on the action result
	if (solution_kept) {
		result = SCIP_FOUNDSOL;
	} else if (result == SCIP_DIDNOTRUN) {
		result = SCIP_DIDNOTFIND;
	}

	// increment the number of search trials spent
	trials_spent++;

	// if all trials are exhausted, or if SCIP should be stopped, stop the search and let SCIP proceed
	if ((trials_spent == static_cast<unsigned int>(trials_per_node)) || SCIPisStopped(scip_ptr)) {
		// reset data for the next time the search is triggered
		trials_spent = 0;
		result = SCIP_DIDNOTRUN;
		// Continue SCIP
		if (!model.solve_iter_continue(result).has_value()) {
			return {true, {}};
		}
	}

	return {false, action_set(model)};
}

}  // namespace ecole::dynamics
