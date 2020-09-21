"""Test Ecole instance generators in Python.

The instance generators are written in python.
This file tests the instance generators with their
default set of parameters.
"""
import numpy as np

import pytest

import ecole


def test_SetCover_next():
    """ Test next() for Setcover instance generator with default parameters. """
    instances = ecole.instances.SetCoverGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_CombinatorialAuction_next():
    """ Test next() for CombinatorialAuction instance generator with default parameters. """
    instances = ecole.instances.CombinatorialAuctionGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_SetCover_iter():
    """ Test iterating over Setcover instance generator with default parameters. """
    instances = ecole.instances.SetCoverGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_CombinatorialAuction_iter():
    """ Test iterating over CombinatorialAuction instance generator with default parameters. """
    instances = ecole.instances.CombinatorialAuctionGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_SetCover_parameter_generator():
    """ Tests the use of a parameter generator for CombinatorialAuction instance generator. """

    class ParameterGenerator:
        def __init__(self):
            self.rng = np.random.RandomState()

        def __next__(self):
            return {
                "nrows": self.rng.randint(450, 550),
                "ncols": self.rng.randint(900, 1100),
            }

        def seed(self, seed):
            self.rng.seed(seed)

    instances = ecole.instances.SetCoverGenerator()
    parameter_generator = ParameterGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = instances.generate_instance(**next(parameter_generator))
        assert isinstance(instance, ecole.core.scip.Model)


def test_CombinatorialAuction_parameter_generator():
    """ Tests the use of a parameter generator for SetCover instance generator. """

    class ParameterGenerator:
        def __init__(self):
            self.rng = np.random.RandomState()

        def __next__(self):
            return {
                "n_items": self.rng.randint(80, 120),
                "n_bids": self.rng.randint(400, 600),
            }

        def seed(self, seed):
            self.rng.seed(seed)

    instances = ecole.instances.CombinatorialAuctionGenerator()
    parameter_generator = ParameterGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = instances.generate_instance(**next(parameter_generator))
        assert isinstance(instance, ecole.core.scip.Model)


def test_SetCover_instance():
    """ Test output of set cover instance. """
    from pyscipopt import Model

    instances = ecole.instances.SetCoverGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert number of rows and columns are consistent with generator
    assert model.getNVars() == instances.ncols
    assert model.getNConss() == instances.nrows

    # assert minimization problem
    assert model.getObjectiveSense() == "minimize"

    # assert all variables are binary
    for var in model.getVars():
        assert var.vtype().isidentifier()

    # assert coefficient of variable in each constraint is 1.
    for constraint in model.getConss():
        assert model.getLhs(constraint) == 1
        for coef in model.getValsLinear(constraint).values():
            assert coef == 1


def test_CombinatorialAuction_instance():
    """ Test output of combinatorial auction instance. """
    from pyscipopt import Model

    instances = ecole.instances.CombinatorialAuctionGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert a variable for each bid
    assert model.getNVars() == instances.n_bids

    # assert minimization problem
    assert model.getObjectiveSense() == "maximize"

    # assert all variables are binary
    for var in model.getVars():
        assert var.vtype().isidentifier()


@pytest.mark.slow
def test_SetCover_solving():
    """ Test that set cover instance solves. """
    from pyscipopt import Model

    instances = ecole.instances.SetCoverGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0


@pytest.mark.slow
def test_CombinatorialAuction_solving():
    """ Test that combinaorial auction instance solves. """
    instances = ecole.instances.CombinatorialAuctionGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0
