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
    instances = ecole.instance.SetCoverGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_CombinatorialAuction_next():
    """ Test next() for CombinatorialAuction instance generator with default parameters. """
    instances = ecole.instance.CombinatorialAuctionGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_CapacitedFacilityLocation_next():
    """ Test next() for CapacitedFaciltyLocation instance generator with default parameters. """
    instances = ecole.instance.CapacitedFacilityLocationGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_IndependentSet_next():
    """ Test next() for IndependentSet instance generator with default parameters. """
    instances = ecole.instance.IndependentSetGenerator()
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instances)
        isinstance(instance, ecole.core.scip.Model)


def test_SetCover_iter():
    """ Test iterating over Setcover instance generator with default parameters. """
    instances = ecole.instance.SetCoverGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_CombinatorialAuction_iter():
    """ Test iterating over CombinatorialAuction instance generator with default parameters. """
    instances = ecole.instance.CombinatorialAuctionGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_CapacitedFacilityLocation_iter():
    """ Test iterating over CapacitedFaciltyLocation instance generator with default parameters. """
    instances = ecole.instance.CapacitedFacilityLocationGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_IndependentSet_iter():
    """ Test iterating over IndependentSet instance generator with default parameters. """
    instances = ecole.instance.IndependentSetGenerator()
    num_episodes = 3

    episode = 0
    for instance in instances:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.core.scip.Model)
        episode += 1


def test_SetCover_instance():
    """ Test output of set cover instance. """
    from pyscipopt import Model

    instances = ecole.instance.SetCoverGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert number of rows and columns are consistent with generator
    assert model.getNVars() == instances.n_cols
    assert model.getNConss() == instances.n_rows

    # assert minimization problem
    assert model.getObjectiveSense() == "minimize"

    # assert all variables are binary
    for var in model.getVars():
        assert var.vtype() == "BINARY"

    # assert coefficient of variable in each constraint is 1.
    for constraint in model.getConss():
        assert model.getLhs(constraint) == 1
        for coef in model.getValsLinear(constraint).values():
            assert coef == 1


def test_CombinatorialAuction_instance():
    """ Test output of combinatorial auction instance. """
    from pyscipopt import Model

    instances = ecole.instance.CombinatorialAuctionGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert a variable for each bid
    assert model.getNVars() == instances.n_bids

    # assert minimization problem
    assert model.getObjectiveSense() == "maximize"

    # assert all variables are binary
    for var in model.getVars():
        assert var.vtype() == "BINARY"


def test_CapacitedFacilityLocation_instance():
    """ Test output of capacited facility location auction instance. """
    from pyscipopt import Model

    instances = ecole.instance.CapacitedFacilityLocationGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert the correct number of variables
    assert (
        model.getNVars() == instances.n_customers * instances.n_facilities + instances.n_facilities
    )

    # assert the correct number of constraints
    num_demand_constraints = instances.n_customers
    num_capacity_constraints = instances.n_facilities
    num_tightening_constraints = instances.n_customers * instances.n_facilities
    assert (
        model.getNConss()
        == num_demand_constraints + num_capacity_constraints + num_tightening_constraints
    )

    # assert minimization problem
    assert model.getObjectiveSense() == "minimize"

    # assert that the correct variables binary/continous
    for var in model.getVars():
        if "y_" in var.name:
            assert var.vtype() == "BINARY"
        else:
            assert var.vtype() == "CONTINUOUS"
            assert var.getLbGlobal() == 0
            assert var.getUbGlobal() == 1

    # assert that constraints have the correct bound and form if applicable
    for constraint in model.getConss():
        if "demand_" in constraint.name:
            assert model.getRhs(constraint) == -1
            for coef in model.getValsLinear(constraint).values():
                assert coef == -1

        elif "capacity_" in constraint.name:
            assert model.getRhs(constraint) == 0

        elif "tightening_" in constraint.name:
            assert model.getRhs(constraint) == 0


def test_IndependentSet_instance():
    """ Test output of independent set instance. """
    from pyscipopt import Model

    instances = ecole.instance.IndependentSetGenerator()
    instance = next(instances)
    model = instance.as_pyscipopt()

    # assert a variable for each node
    assert model.getNVars() == instances.n_nodes

    # assert minimization problem
    assert model.getObjectiveSense() == "maximize"

    # assert all variables are binary
    for var in model.getVars():
        assert var.vtype() == "BINARY"

    # assert coefficient of variable in each constraint is 1.
    for constraint in model.getConss():
        assert model.getRhs(constraint) == 1
        for coef in model.getValsLinear(constraint).values():
            assert coef == 1


@pytest.mark.slow
def test_SetCover_solving():
    """ Test that set cover instance solves. """
    from pyscipopt import Model

    instances = ecole.instance.SetCoverGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0


@pytest.mark.slow
def test_CombinatorialAuction_solving():
    """ Test that combinaorial auction instance solves. """
    instances = ecole.instance.CombinatorialAuctionGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0


@pytest.mark.slow
def test_CapacitedFacilityLocation_solving():
    """ Test that capacited facility location instance solves. """
    from pyscipopt import Model

    instances = ecole.instance.CapacitedFacilityLocationGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0


@pytest.mark.slow
def test_IndependentSet_solving():
    """ Test that independent set instance solves. """
    instances = ecole.instance.IndependentSetGenerator()
    instance = next(instances)

    model = instance.as_pyscipopt()
    model.optimize()

    assert model.getGap() == 0
