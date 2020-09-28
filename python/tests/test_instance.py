"""Test Ecole instance generators in Python.

The instance generators are written in python.
This file tests the instance generators with their default set of parameters.
"""

import importlib.util

import numpy as np
import pytest

import ecole
import ecole.instance as I


requires_pyscipopt = pytest.mark.skipif(
    importlib.util.find_spec("pyscipopt") is None, reason="PyScipOpt is not installed.",
)


def pytest_generate_tests(metafunc):
    """Parametrize the `instance_generator` fixture.

    Add instance generators here to have them automatically run all the tests that take
    `instance_generator` as input.
    """
    if "instance_generator" in metafunc.fixturenames:
        all_instance_generators = (
            I.SetCoverGenerator(n_rows=100, n_cols=200),
            I.CombinatorialAuctionGenerator(n_items=50, n_bids=150),
            I.CapacitedFacilityLocationGenerator(n_customers=60, n_facilities=50),
            I.IndependentSetGenerator(n_nodes=100),
        )
        metafunc.parametrize("instance_generator", all_instance_generators)


def test_generator_next(instance_generator):
    """Generate new instance with next."""
    num_episodes = 3

    for episode in range(num_episodes):
        instance = next(instance_generator)
        isinstance(instance, ecole.scip.Model)


def test_generator_iter(instance_generator):
    """Generate multiple instances by iterations"""
    num_episodes = 3

    episode = 0
    for instance in instance_generator:
        if episode == num_episodes:
            break
        assert isinstance(instance, ecole.scip.Model)
        episode += 1


@pytest.mark.slow
def test_generator_instance_solving(instance_generator):
    """Instances generated are valid."""
    model = next(instance_generator)
    model.solve()
    assert model.is_solved()


@requires_pyscipopt
def test_SetCover_instance():
    """Test output of set cover instance."""
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


@requires_pyscipopt
def test_CombinatorialAuction_instance():
    """Test output of combinatorial auction instance."""
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


@requires_pyscipopt
def test_CapacitedFacilityLocation_instance():
    """Test output of capacited facility location auction instance."""
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


@requires_pyscipopt
def test_IndependentSet_instance():
    """Test output of independent set instance."""
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
