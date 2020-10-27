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
            I.CapacitatedFacilityLocationGenerator(n_customers=60, n_facilities=50),
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
