import numpy as np

import ecole.scip


class SetCoverGenerator:
    def __init__(
        self, n_rows: int = 500, n_cols: int = 1000, density: float = 0.05, max_coef: int = 100
    ):
        """Constructor for the set cover generator.

        The parameters passed in this constructor will be used when a user calls next() or iterates
        over the object.

        Parameters
        ----------
        n_rows:
            The number of rows.
        n_cols:
            The number of columns.
        density:
            The density of the constraint matrix.
            The value must be in the range (0,1].
        max_coef:
            Maximum objective coefficient.
            The value must be in >= 1.

        """
        self.n_rows = n_rows
        self.n_cols = n_cols
        self.density = density
        self.max_coef = max_coef

        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        """ Gets the next instances of a set covering problem.

        This method calls generate_instance() with the parameters passed in
        the constructor and returns the ecole.scip.Model.

        Returns
        -------
        model:
            an ecole model of a set cover instance.

        """
        return generate_instance(self.n_rows, self.n_cols, self.density, self.max_coef, self.rng)

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
    n_rows: int, n_cols: int, density: float, max_coef: int, rng: np.random.RandomState
):
    """Generates an instance of a combinatorial auction problem.

    This method generates an instance of a combinatorial auction problem based on the
    specified parameters and returns it as an ecole model.

    Algorithm described in:
        E.Balas and A.Ho, Set covering algorithms using cutting planes, heuristics,
        and subgradient optimization: A computational study, Mathematical
        Programming, 12 (1980), 37-60.

    Parameters
    ----------
    n_rows:
        The number of rows.
    n_cols:
        The number of columns.
    density:
        The density of the constraint matrix.
        The value must be in the range (0,1].
    max_coef:
        Maximum objective coefficient.
        The value must be in >= 1.

    Returns
    -------
    model:
        an ecole model of a set cover instance.

    """
    nnzrs = int(n_rows * n_cols * density)

    assert nnzrs >= n_rows  # at least 1 col per row
    assert nnzrs >= 2 * n_cols  # at leats 2 rows per col

    indices = np.empty((nnzrs,), dtype=int)

    # sample column indices
    indices[: 2 * n_cols] = np.arange(2 * n_cols) % n_cols  # force at leats 2 rows per col
    indices[2 * n_cols :] = (
        rng.choice(n_cols * (n_rows - 2), size=nnzrs - (2 * n_cols), replace=False) % n_cols
    )  # remaining column indexes are random

    # count the resulting number of rows, for each column
    _, col_n_rows = np.unique(indices, return_counts=True)

    # for each column, sample row indices
    i = 0
    indptr = [0]
    indices[:n_rows] = rng.permutation(n_rows)  # pre-fill to force at least 1 column per row
    for n in col_n_rows:

        # column is already filled, nothing to do
        if i + n <= n_rows:
            pass

        # column is empty, fill with random rows
        elif i >= n_rows:
            indices[i : i + n] = rng.choice(n_rows, size=n, replace=False)

        # column is partially filled, complete with random rows among remaining ones
        elif i + n > n_rows:
            remaining_rows = np.setdiff1d(np.arange(n_rows), indices[i:n_rows], assume_unique=True)
            indices[n_rows : i + n] = rng.choice(remaining_rows, size=i + n - n_rows, replace=False)

        i += n
        indptr.append(i)

    # sample objective coefficients
    c = rng.randint(max_coef, size=n_cols) + 1

    # convert csc indices/indptr to csr indices/indptr
    indptr_csr = np.zeros((n_rows + 1), dtype=int)
    indptr_counter = np.zeros((n_rows + 1), dtype=int)
    indices_csr = np.zeros(len(indices), dtype=int)

    # compute indptr for csr
    for i in range(len(indices)):
        indptr_csr[indices[i] + 1] += 1
    indptr_csr = np.cumsum(indptr_csr)

    # compute indices for csr
    for col in range(n_cols):
        for row in indices[indptr[col] : indptr[col + 1]]:
            indices_csr[indptr_csr[row] + indptr_counter[row]] = col
            indptr_counter[row] += 1

    # generate SCIP instance from problem
    from pyscipopt import Model

    model = Model()
    model.setMinimize()

    # add variables
    for j in range(n_cols):
        model.addVar(name=f"x{j+1}", vtype="B", obj=c[j])

    # add constraints
    model_vars = model.getVars()
    for i in range(n_rows):
        cons_lhs = 0
        consvars = [model_vars[j] for j in indices_csr[indptr_csr[i] : indptr_csr[i + 1]]]
        for var in consvars:
            cons_lhs += var
        model.addCons(cons_lhs >= 1)

    return ecole.scip.Model.from_pyscipopt(model)
