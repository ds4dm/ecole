#include <algorithm>
#include <cstddef>

#include <range/v3/view/zip.hpp>
#include <scip/cons_linear.h>
#include <scip/scip.h>

#include "ecole/scip/cons.hpp"

#include "unit-tests.hpp"

namespace views = ranges::views;

namespace ecole::instance {

namespace {

auto same_constraint_permutation(SCIP* scip1, SCIP_CONS* constraint1, SCIP* scip2, SCIP_Cons* constraint2) noexcept
	-> bool {
	if (scip::cons_get_lhs(scip1, constraint1) != scip::cons_get_lhs(scip2, constraint2)) {
		return false;
	}
	if (scip::cons_get_rhs(scip1, constraint1) != scip::cons_get_rhs(scip2, constraint2)) {
		return false;
	}

	auto vals1 = scip::get_vals_linear(scip1, constraint1);
	auto vals2 = scip::get_vals_linear(scip2, constraint2);
	if (vals1.size() != vals2.size()) {
		return false;
	}
	for (auto [v1, v2] : views::zip(vals1, vals2)) {
		if (v1 != v2) {
			return false;
		}
	}

	auto const vars1 = scip::get_vars_linear(scip1, constraint1);
	auto const vars2 = scip::get_vars_linear(scip2, constraint2);
	auto const zipped_vars = views::zip(vars1, vars2);
	return std::all_of(zipped_vars.begin(), zipped_vars.end(), [](auto const& var_pair) {
		return SCIPvarGetIndex(var_pair.first) == SCIPvarGetIndex(var_pair.second);
	});
}

}  // namespace

auto same_problem_permutation(scip::Model const& model1, scip::Model const& model2) noexcept -> bool {
	auto* scip1 = const_cast<SCIP*>(model1.get_scip_ptr());
	auto* scip2 = const_cast<SCIP*>(model2.get_scip_ptr());

	if (SCIPgetObjsense(scip1) != SCIPgetObjsense(scip2)) {
		return false;
	}

	if (model1.variables().size() != model2.variables().size()) {
		return false;
	}

	auto const constraints1 = model1.constraints();
	auto const constraints2 = model2.constraints();
	if (constraints1.size() != constraints2.size()) {
		return false;
	}
	for (std::size_t i = 0; i < constraints1.size(); ++i) {
		if (!same_constraint_permutation(scip1, constraints1[i], scip2, constraints2[i])) {
			return false;
		}
	}

	return true;
}

}  // namespace ecole::instance
