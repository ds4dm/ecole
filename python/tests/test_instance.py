"""Test Ecole instance generators in Python.

Most instance generartors are written in Ecole C++ library.
This is where the logic should be tested.
The tests provided here run the same assertions on all generators.
They mostly test that the code Bindings work as expected.
"""

import itertools

import pytest

import ecole


def pytest_generate_tests(metafunc):
    """Parametrize the `instance_generator` fixture.

    Add instance generator here to have them automatically run all the tests that take
    `instance_generator` as input.
    """
    if "instance_generator" in metafunc.fixturenames:
        all_instance_generators = (
            ecole.instance.SetCoverGenerator(n_rows=100, n_cols=200),
            ecole.instance.CombinatorialAuctionGenerator(n_items=50, n_bids=150),
            ecole.instance.IndependentSetGenerator(n_nodes=100),
            ecole.instance.CapacitatedFacilityLocationGenerator(n_customers=60, n_facilities=50),
        )
        metafunc.parametrize("instance_generator", all_instance_generators)


def test_default_init(instance_generator):
    """Construct with default arguments."""
    type(instance_generator)()


def test_random_engine_init(instance_generator):
    """Construct a random engine."""
    type(instance_generator)(random_engine=ecole.RandomEngine())


def test_generate_instance(instance_generator):
    """Use stateless instance generating function."""
    InstanceGenerator = type(instance_generator)
    model = InstanceGenerator.generate_instance(random_engine=ecole.RandomEngine())
    assert isinstance(model, ecole.scip.Model)


def test_infinite_iteration(instance_generator):
    """For loop, even if infinite, can iterate over the iterator."""
    for model in instance_generator:
        assert isinstance(model, ecole.scip.Model)
        break


def test_repeated_slice_iteration(instance_generator):
    """Generate a finite number of instances in multiple epochs."""
    for epoch in range(2):
        for model in itertools.islice(instance_generator, 2):
            assert isinstance(model, ecole.scip.Model)


def test_SetCoverGenerator_parameters():
    """Parameters are bound in the constructor and as attributes."""
    generator = ecole.instance.SetCoverGenerator(n_cols=10)
    assert generator.n_cols == 10


def test_IndependentSetGenerator_parameters():
    """Parameters are bound in the constructor and as attributes."""
    generator = ecole.instance.IndependentSetGenerator(graph_type="erdos_renyi")
    assert generator.graph_type.name == "erdos_renyi"


def test_CombinatorialAuctionGenerator_parameters():
    """Parameters are bound in the constructor and as attributes."""
    generator = ecole.instance.CombinatorialAuctionGenerator(additivity=-1)
    assert generator.additivity == -1


def test_CapacitatedFacilityLocationGenerator_parameters():
    """Parameters are bound in the constructor and as attributes."""
    generator = ecole.instance.CapacitatedFacilityLocationGenerator(
        ratio=-1, demand_interval=(1, 5)
    )
    assert generator.ratio == -1
    assert generator.demand_interval == (1, 5)
