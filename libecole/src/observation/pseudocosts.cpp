#include <cmath>
#include <cstddef>
#include <optional>

#include <nonstd/span.hpp>
#include <range/v3/view/zip.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/pseudocosts.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

namespace views = ranges::views;

namespace {

/** get vanilla full strong branching scores and variables */
auto scip_get_lp_branch_cands(SCIP* const scip) noexcept {
	SCIP_VAR** cands = nullptr;
	SCIP_Real* cands_lp_values = nullptr;
	int n_cands = 0;
	SCIPgetLPBranchCands(scip, &cands, &cands_lp_values, nullptr, nullptr, &n_cands, nullptr);
	return std::tuple{
		nonstd::span{cands, static_cast<std::size_t>(n_cands)},
		nonstd::span{cands_lp_values, static_cast<std::size_t>(n_cands)},
	};
}

}  // namespace

std::optional<xt::xtensor<double, 1>> Pseudocosts::extract(scip::Model& model, bool /* done */) {
	if (model.stage() != SCIP_STAGE_SOLVING) {
		return {};
	}

	auto* const scip = model.get_scip_ptr();
	auto const [cands, lp_values] = scip_get_lp_branch_cands(scip);

	/* Store pseudocosts in tensor */
	auto const nb_vars = static_cast<std::size_t>(SCIPgetNVars(scip));
	xt::xtensor<double, 1> pseudocosts({nb_vars}, std::nan(""));

	for (auto const [var, lp_val] : views::zip(cands, lp_values)) {
		auto const var_index = static_cast<std::size_t>(SCIPvarGetProbindex(var));
		auto const score = SCIPgetVarPseudocostScore(scip, var, lp_val);
		pseudocosts[var_index] = static_cast<double>(score);
	}

	return pseudocosts;
}

}  // namespace ecole::observation
