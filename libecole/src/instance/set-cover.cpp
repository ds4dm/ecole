#include <fmt/format.h>
#include <map>

#include <xtensor/xrandom.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/instance/set-cover.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"

namespace ecole::instance {

/*************************************
 *  SetCoverGenerator methods  *
 *************************************/

SetCoverGenerator::SetCoverGenerator(Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{parameters_} {}
SetCoverGenerator::SetCoverGenerator(Parameters parameters_) :
	SetCoverGenerator{parameters_, ecole::spawn_random_engine()} {}
SetCoverGenerator::SetCoverGenerator() : SetCoverGenerator(Parameters{}) {}

scip::Model SetCoverGenerator::next() {
	return generate_instance(parameters, random_engine);
}

void SetCoverGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

namespace {

using std::size_t;
using xvector = xt::xtensor<size_t, 1>;

/** Sets a slice of a 1-D xtensor.
 *
 * Sets the the slice from start_index to end_index of vector a to
 * vector b.  Vector b should be of size (end_index - start_index).
 */
auto set_slice(xvector& a, xvector& b, size_t start_index, size_t end_index) {
	auto slice = xt::view(a, xt::range(start_index, end_index));
	slice = b;
}

/** Gets a slice of a 1-D xtensor.
 *
 * Gets the the slice from start_index to end_index of vector a.
 */
auto get_slice(xvector& a, size_t start_index, size_t end_index) -> xvector {
	auto slice = xt::view(a, xt::range(start_index, end_index));
	return slice;
}

/** Gets the counts of each element in a 1-D xtensor.
 *
 * This method is specifically for the case where
 * each each element is in the range [0, n_cols].
 */
auto get_counts(xvector& indices, size_t n_cols) -> xvector {
	xvector counts({n_cols}, 0);
	for (size_t i = 0; i < indices.size(); ++i) {
		++counts(indices[i]);
	}
	return counts;
}

/** Samples values in a range and returns them as a 1-D xtensor.
 *
 * Samples num_samples values in the range from start_index to
 * end_index.
 */
auto get_choice_in_range(size_t start_index, size_t end_index, size_t num_samples, RandomEngine& random_engine)
	-> xvector {
	xvector choices = xt::arange<size_t>(start_index, end_index, 1);
	xvector samples = xt::random::choice(choices, num_samples, false, random_engine);
	return samples;
}

/** Adds a varaible to the SCIP Model.
 *
 * Adds a single binary variable with a specified cost.
 */
auto add_var(SCIP* scip, size_t idx, SCIP_Real cost) -> SCIP_VAR* {
	auto const name = fmt::format("x_{}", idx);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., cost, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Adds all varaibles to the SCIP Model.
 *
 * A variable is added for each element (or column) in the
 * set cover problem.
 */
auto add_vars(SCIP* scip, xt::xtensor<SCIP_Real, 1> const& c) -> xt::xtensor<SCIP_VAR*, 1> {
	auto vars = xt::xtensor<SCIP_VAR*, 1>{{c.shape()}};
	for (size_t i = 0; i < c.size(); ++i) {
		vars(i) = add_var(scip, i, c(i));
	}
	return vars;
}

/** Adds set covering constaints.
 *
 * For each set, at least one element is required to the solution.
 */
auto add_constaints(SCIP* scip, xt::xtensor<SCIP_VAR*, 1> vars, xvector& indices, xvector& indptr, size_t n_rows) {

	for (size_t i = 0; i < n_rows; ++i) {

		// get the set of variables to be added in the constraint.
		auto cons_vars = xt::xtensor<SCIP_VAR*, 1>{{indptr(i + 1) - indptr(i)}};
		for (size_t j = indptr(i); j < indptr(i + 1); ++j) {
			cons_vars(j - indptr(i)) = vars(indices(j));
		}

		// add constraint to SCIP Model.
		auto name = fmt::format("c_{}", i);
		auto const inf = SCIPinfinity(scip);
		auto coefs = xt::xtensor<SCIP_Real, 1>(cons_vars.shape(), 1.);
		auto cons =
			scip::create_cons_basic_linear(scip, name.c_str(), cons_vars.size(), &cons_vars(0), coefs.data(), 1.0, inf);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

/** Convert CSC sparse indicies and index pointers to CSR.
 *
 * This implementation only converts the indices and points,
 * and does not consider the values as they are all 1.  A
 * tuple of the indices and index pointers in CSR format are
 * returned.
 */
auto convert_csc_to_csr(xvector& indices, xvector& indptr, size_t n_rows, size_t n_cols) {

	xvector indptr_csr({n_rows + 1}, 0);
	xvector indices_csr({indices.size()}, 0);

	for (size_t j = 0; j < indices.size(); ++j) {
		indptr_csr(indices(j) + 1) += 1;
	}
	indptr_csr = xt::cumsum(indptr_csr);

	for (size_t col = 0; col < n_cols; ++col) {
		for (size_t jj = indptr(col); jj < indptr(col + 1); ++jj) {
			size_t row = indices(jj);
			indices_csr(indptr_csr(row)) = col;
			indptr_csr(row) = indptr_csr(row) + 1;
		}
	}

	size_t last = 0;
	for (size_t row = 0; row <= n_rows; ++row) {
		std::swap(last, indptr_csr[row]);
	}

	return std::make_tuple(indptr_csr, indices_csr);
}

}  // namespace

/******************************************
 *  SetCoverGenerator::generate_instance  *
 ******************************************/

scip::Model SetCoverGenerator::generate_instance(Parameters parameters, RandomEngine& random_engine) {

	auto const n_rows = parameters.n_rows;
	auto const n_cols = parameters.n_cols;
	auto const density = parameters.density;
	auto const max_coef = static_cast<size_t>(parameters.max_coef);

	auto const nnzrs = static_cast<size_t>(static_cast<double>(n_rows * n_cols) * density);

	xvector indices({nnzrs}, 0);

	// force at least 2 rows per col
	xvector first_indices = xt::arange<size_t>(0, 2 * n_cols, 1) % n_cols;
	set_slice(indices, first_indices, 0, 2 * n_cols);

	// assign remaining column indexes at random
	xvector samples = get_choice_in_range(0, n_cols * (n_rows - 2), nnzrs - (2 * n_cols), random_engine) % n_cols;
	set_slice(indices, samples, 2 * n_cols, nnzrs);

	// get counts of unique elements
	auto col_n_rows = get_counts(indices, n_cols);

	// ensure at least 1 column per row
	auto perm = xt::random::permutation<size_t>(n_rows, random_engine);
	set_slice(indices, perm, 0, n_rows);

	size_t i = 0;
	xvector indptr({n_cols + 1}, 0);
	size_t indptr_idx = 1;

	for (size_t idx = 0; idx < col_n_rows.size(); ++idx) {
		size_t n = col_n_rows(idx);

		if (i + n <= n_rows) {
		} else if (i >= n_rows) {
			xvector sampled_rows = get_choice_in_range(0, n_rows, n, random_engine);
			set_slice(indices, sampled_rows, i, i + n);

		} else if (i + n > n_rows) {
			auto remaining_rows = xt::setdiff1d(xt::arange<size_t>(0, n_rows, 1), get_slice(indices, i, n_rows));
			xvector choices = xt::random::choice(remaining_rows, i + n - n_rows, false, random_engine);
			set_slice(indices, choices, n_rows, i + n);
		}

		i += n;
		indptr(indptr_idx) = i;
		++indptr_idx;
	}

	// convert csc indices/ptrs to csr
	auto [indptr_csr, indices_csr] = convert_csc_to_csr(indices, indptr, n_rows, n_cols);

	// sample coefficients
	xt::xtensor<SCIP_Real, 1> c = xt::random::randint<size_t>({n_cols}, 0, max_coef, random_engine);

	// create scip model
	auto model = scip::Model::prob_basic();
	model.set_name(fmt::format("SetCover-{}-{}", parameters.n_rows, parameters.n_cols));
	auto* const scip = model.get_scip_ptr();
	scip::call(SCIPsetObjsense, scip, SCIP_OBJSENSE_MINIMIZE);

	// add variables and constraints
	auto const vars = add_vars(scip, c);
	add_constaints(scip, vars, indices_csr, indptr_csr, n_rows);

	return model;

}  // generate_instance

}  // namespace ecole::instance
