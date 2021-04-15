#pragma once

#include <utility>

#include <scip/scip.h>

#include "branching/lambda-branchrule.hpp"

namespace ecole::scip {

namespace internal {

class IndexBranchruleFunc {
public:
	IndexBranchruleFunc(std::size_t branching_index) noexcept : m_branching_index(branching_index) {}

	auto operator()(SCIP* scip) const noexcept {
		SCIP_VAR** branch_cands = nullptr;
		int n_cands [[maybe_unused]] = 0;
		auto retcode
			[[maybe_unused]] = SCIPgetLPBranchCands(scip, &branch_cands, nullptr, nullptr, &n_cands, nullptr, nullptr);
		assert(n_cands > 0);
		assert(retcode == SCIP_OKAY);
		retcode = SCIPbranchVar(scip, branch_cands[m_branching_index], nullptr, nullptr, nullptr);
		assert(retcode == SCIP_OKAY);
		return SCIP_BRANCHED;
	}

private:
	std::size_t m_branching_index;
};

}  // namespace internal

class IndexBranchrule : public LambdaBranchrule<internal::IndexBranchruleFunc> {
public:
	IndexBranchrule(SCIP* scip, const char* name, std::size_t branching_index) noexcept :
		LambdaBranchrule(scip, name, {branching_index}) {}
};

}  // namespace ecole::scip
