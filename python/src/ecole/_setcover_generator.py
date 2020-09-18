import numpy as np

import ecole.scip


class SetCoverGenerator:
    def __init__(
        self, nrows: int = 500, ncols: int = 1000, density: float = 0.05, max_coef: int = 100
    ):
        """Constructor for the set cover generator.

        The parameters passed in this constructor will be used when a user calls next().  In order to modify
        parameters between instances, see generate_instances.

        Parameters
        ----------
        nrows
            Desired number of rows
        ncols
            Desired number of columns
        density:
            Desired density of the constraint matrix.
            The value must be in the range Value in the range (0,1].
        max_coef:
            Maximum objective coefficient.
            The value must be in >= 1.

        """
        self.nrows = nrows
        self.ncols = ncols
        self.density = density
        self.max_coef = max_coef

        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        """ Generates an instance of setcover.

        This method is used to generate an instance of the setcover problem
        with the set of parameters passed in the constructor.

        Returns
        -------
        model:
            an ecole model of a set cover instance.

        """
        return self.generate_instance(self.nrows, self.ncols, self.density, self.max_coef)

    def seed(self, seed: int):
        """ Seeds SetCoverGenerator.

        This method sets the random seed of the SetCoverGenerator.

        Parameters
        ----------
        seed:
            The seed in which to set the random number generator with.

        """
        self.rng.seed(seed)

    def generate_instance(
        self, nrows: int = 500, ncols: int = 1000, density: float = 0.05, max_coef: int = 100
    ):
        """Generates an instance of a combinatorial auction problem.

        This method generates a random instance of a combinatorial auction problem based on the
        specified parameters and returns it as an ecole model.  The user can call this function
        with any set of parameters or simply use next() which call this method with the set of
        parameters in the constructor.

        Generate a set cover instance with specified characteristics, and writes
        it to a file in the LP format.
        Algorithm described in:
            E.Balas and A.Ho, Set covering algorithms using cutting planes, heuristics,
            and subgradient optimization: A computational study, Mathematical
            Programming, 12 (1980), 37-60.

        Parameters
        ----------
        nrows
            Desired number of rows
        ncols
            Desired number of columns
        density:
            Desired density of the constraint matrix.
            The value must be in the range Value in the range (0,1].
        max_coef:
            Maximum objective coefficient.
            The value must be in >= 1.

        Returns
        -------
        model:
            an ecole model of a set cover instance.

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

        # convert csc indices/indptr to csr indices/indptr
        indptr_csr = np.zeros((nrows + 1), dtype=int)
        indptr_counter = np.zeros((nrows + 1), dtype=int)
        indices_csr = np.zeros(len(indices), dtype=int)

        # compute indptr for csr
        for i in range(len(indices)):
            indptr_csr[indices[i] + 1] += 1
        indptr_csr = np.cumsum(indptr_csr)

        # compute indices for csr
        for col in range(ncols):
            for row in indices[indptr[col] : indptr[col + 1]]:
                indices_csr[indptr_csr[row] + indptr_counter[row]] = col
                indptr_counter[row] += 1

        # generate SCIP instance from problem
        from pyscipopt import Model

        model = Model()
        model.setMinimize()

        # add variables
        for j in range(ncols):
            model.addVar(name=f"x{j+1}", vtype="B", obj=c[j])

        # add constraints
        model_vars = model.getVars()
        for i in range(nrows):
            cons_lhs = 0
            consvars = [model_vars[j] for j in indices_csr[indptr_csr[i] : indptr_csr[i + 1]]]
            for var in consvars:
                cons_lhs += var
            model.addCons(cons_lhs >= 1)

        return ecole.scip.Model.from_pyscipopt(model)
