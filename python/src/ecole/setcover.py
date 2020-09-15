import numpy as np
import scipy
import scipy.sparse

from pyscipopt import Model

import ecole.scip


class Setcover:
    def __init__(self, parameter_generator):
        self.parameter_generator = parameter_generator
        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        param_dict = next(self.parameter_generator)
        model = self.generate_instance(**param_dict)

        return ecole.scip.Model.from_pyscipopt(model)

    def seed(self, seed):
        self.rng.seed(seed)

    def generate_instance(self, nrows=500, ncols=1000, density=0.05, max_coef=100):
        """
		Generate a set cover instance with specified characteristics, and writes
		it to a file in the LP format.
		Algorithm described in:
			E.Balas and A.Ho, Set covering algorithms using cutting planes, heuristics,
			and subgradient optimization: A computational study, Mathematical
			Programming, 12 (1980), 37-60.
		Parameters
		----------
		nrows : int
			Desired number of rows
		ncols : int
			Desired number of columns
		density: float between 0 (excluded) and 1 (included)
			Desired density of the constraint matrix
		max_coef: int
			Maximum objective coefficient (>=1)
		"""
        nnzrs = int(nrows * ncols * density)

        assert nnzrs >= nrows  # at least 1 col per row
        assert nnzrs >= 2 * ncols  # at leats 2 rows per col

        indices = np.empty((nnzrs,), dtype=int)

        # sample column indices
        indices[: 2 * ncols] = np.arange(2 * ncols) % ncols  # force at leats 2 rows per col
        indices[2 * ncols :] = (
            self.rng.choice(ncols * (nrows - 2), size=nnzrs - (2 * ncols), replace=False) % ncols
        )  # remaining column indexes are random

        # count the resulting number of rows, for each column
        _, col_nrows = np.unique(indices, return_counts=True)

        # for each column, sample row indices
        i = 0
        indptr = [0]
        indices[:nrows] = self.rng.permutation(nrows)  # pre-fill to force at least 1 column per row
        for n in col_nrows:

            # column is already filled, nothing to do
            if i + n <= nrows:
                pass

            # column is empty, fill with random rows
            elif i >= nrows:
                indices[i : i + n] = self.rng.choice(nrows, size=n, replace=False)

            # column is partially filled, complete with random rows among remaining ones
            elif i + n > nrows:
                remaining_rows = np.setdiff1d(
                    np.arange(nrows), indices[i:nrows], assume_unique=True
                )
                indices[nrows : i + n] = self.rng.choice(
                    remaining_rows, size=i + n - nrows, replace=False
                )

            i += n
            indptr.append(i)

        # sample objective coefficients
        c = self.rng.randint(max_coef, size=ncols) + 1

        # sparse CSC to sparse CSR matrix
        A = scipy.sparse.csc_matrix(
            (np.ones(len(indices), dtype=int), indices, indptr), shape=(nrows, ncols)
        ).tocsr()
        indices = A.indices
        indptr = A.indptr

        # generate SCIP instance from problem
        model = Model()
        model.setMinimize()

        # add variables
        for j in range(ncols):
            model.addVar(name=f"x{j+1}", vtype="B", obj=c[j])

        # add constraints
        model_vars = model.getVars()
        for i in range(nrows):
            cons_lhs = 0
            consvars = [model_vars[j] for j in indices[indptr[i] : indptr[i + 1]]]
            for var in consvars:
                cons_lhs += var
            model.addCons(cons_lhs >= 1)

        return model
