import numpy as np

import ecole.scip


class CapacitedFacilityLocationGenerator:
    def __init__(self, n_customers: int = 100, n_facilities: int = 100, ratio: float = 5.0):
        """Constructor for the capacited facility location generator.

        The parameters passed in this constructor will be used when a user calls next() or iterates
        over the object.

        Parameters:
        ----------
        n_customers
            The number of customers.
        n_facilities
            The number of facilities.
        ratio:
            The ratio of capacity/demand.

        """
        self.n_customers = n_customers
        self.n_facilities = n_facilities
        self.ratio = ratio

        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        """ Gets the next instances of a capacited facility location problem.

        This method calls generate_instance() with the parameters passed in
        the constructor and returns the ecole.scip.Model.

        Returns
        -------
        model:
            an ecole model of a capacited facility location instance.

        """
        return generate_instance(self.n_customers, self.n_facilities, self.ratio, self.rng)

    def seed(self, seed: int):
        """ Seeds CapacitedFacilityLocationGenerator.

        This method sets the random seed of the CapacitedFacilityLocationGenerator.

        Parameters
        ----------
        seed:
            The seed in which to set the random number generator with.

        """
        self.rng.seed(seed)


def generate_instance(
    n_customers: int, n_facilities: int, ratio: float, rng: np.random.RandomState
):
    """ Generates an instance of a capacited facility location problem.

    This method generates an instance of the capacited facility location problem based on the
    specified parameters and returns it as an ecole model.

    The problem is generated following:
        Cornuejols G, Sridharan R, Thizy J-M (1991)
        A Comparison of Heuristics and Relaxations for the Capacitated Plant Location Problem.
        European Journal of Operations Research 50:280-297.

    Parameters
    ----------
    n_customers:
        The number of customers.
    n_facilities:
        The number of facilities.
    ratio:
        The ratio of capacity/demand.
    rng:
        A random number generator.

    Returns
    -------
    model:
        an ecole model of a independent set instance.

    """
    c_x = rng.rand(n_customers)
    c_y = rng.rand(n_customers)

    f_x = rng.rand(n_facilities)
    f_y = rng.rand(n_facilities)

    demands = rng.randint(5, 35 + 1, size=n_customers)
    capacities = rng.randint(10, 160 + 1, size=n_facilities)
    fixed_costs = rng.randint(100, 110 + 1, size=n_facilities) * np.sqrt(capacities) + rng.randint(
        90 + 1, size=n_facilities
    )
    fixed_costs = fixed_costs.astype(int)

    total_demand = demands.sum()
    total_capacity = capacities.sum()

    # adjust capacities according to ratio
    capacities = capacities * ratio * total_demand / total_capacity
    capacities = capacities.astype(int)
    total_capacity = capacities.sum()

    # transportation costs
    trans_costs = (
        np.sqrt(
            (c_x.reshape((-1, 1)) - f_x.reshape((1, -1))) ** 2
            + (c_y.reshape((-1, 1)) - f_y.reshape((1, -1))) ** 2
        )
        * 10
        * demands.reshape((-1, 1))
    )

    # write problem as pyscipopt model
    from pyscipopt import Model

    model = Model()
    model.setMinimize()

    # add variables
    var_dict = {}
    var_dict["x"] = {}

    # transport costs from facility to locations
    for i in range(n_customers):
        var_dict["x"][i + 1] = {}
        for j in range(n_facilities):
            var_dict["x"][i + 1][j + 1] = model.addVar(
                name=f"x_{i+1}_{j+1}", vtype="C", obj=trans_costs[i, j], lb=0, ub=1,
            )

    # fixed costs for opening facilities
    var_dict["y"] = {}
    for j in range(n_facilities):
        var_dict["y"][j + 1] = model.addVar(name=f"y_{j+1}", vtype="B", obj=fixed_costs[j])

    # add constraints
    # constraint for each customer to have demand
    for i in range(n_customers):
        vars_to_sum = [-var_dict["x"][i + 1][j + 1] for j in range(n_facilities)]
        cons_lhs = 0
        for var in vars_to_sum:
            cons_lhs += var
        model.addCons(cons_lhs <= -1, name=f"demand_{i+1}")

    # constraint that the demand at each location does not exceed capacity
    for j in range(n_facilities):
        vars_to_sum = [demands[i] * var_dict["x"][i + 1][j + 1] for i in range(n_customers)]
        vars_to_sum.append(-capacities[j] * var_dict["y"][j + 1])
        cons_lhs = 0
        for var in vars_to_sum:
            cons_lhs += var
        model.addCons(cons_lhs <= 0, name=f"capacity_{i+1}")

    # constraint to for LP relaxation tightening
    for i in range(n_customers):
        for j in range(n_facilities):
            model.addCons(
                var_dict["x"][i + 1][j + 1] - var_dict["y"][j + 1] <= 0,
                name=f"tightening_{i+1}_{j+1}",
            )

    return ecole.scip.Model.from_pyscipopt(model)
