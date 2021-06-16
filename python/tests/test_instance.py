"""Test Ecole instance generators in Python.

Most instance generartors are written in Ecole C++ library.
This is where the logic should be tested.
The tests provided here run the same assertions on all generators.
They mostly test that the code Bindings work as expected.
"""

import itertools

import pytest

import ecole


@pytest.fixture(scope="module")
def tmp_dataset(tmp_path_factory, problem_file):
    """Create a local dataset of problem files."""
    model = ecole.scip.Model.from_file(str(problem_file))
    path = tmp_path_factory.mktemp("instances")
    for name in "abc":
        model.write_problem(str(path / f"model-{name}.lp"))
    return path


@pytest.fixture(
    params=(
        ecole.instance.FileGenerator,
        ecole.instance.SetCoverGenerator,
        ecole.instance.CombinatorialAuctionGenerator,
        ecole.instance.IndependentSetGenerator,
        ecole.instance.CapacitatedFacilityLocationGenerator,
    )
)
def instance_generator(request, tmp_dataset):
    """Fixture to run tests with various instance generators."""
    args = {
        ecole.instance.FileGenerator: {"directory": str(tmp_dataset)},
        ecole.instance.SetCoverGenerator: {"n_rows": 100, "n_cols": 200},
        ecole.instance.CombinatorialAuctionGenerator: {"n_items": 50, "n_bids": 150},
        ecole.instance.IndependentSetGenerator: {"n_nodes": 100},
        ecole.instance.CapacitatedFacilityLocationGenerator: {
            "n_customers": 60,
            "n_facilities": 50,
        },
    }
    return request.param(**args[request.param])


def test_default_init(instance_generator):
    """Construct with default arguments."""
    if isinstance(instance_generator, ecole.instance.FileGenerator):
        pytest.skip("No dataset in default directory")
    type(instance_generator)()


def test_random_engine_init(instance_generator):
    """Construct a random engine."""
    if isinstance(instance_generator, ecole.instance.FileGenerator):
        pytest.skip("No dataset in default directory")
    type(instance_generator)(random_engine=ecole.RandomEngine())


def test_generate_instance(instance_generator):
    """Use stateless instance generating function."""
    if isinstance(instance_generator, ecole.instance.FileGenerator):
        pytest.skip("No generate_instance for file loaders")
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


def test_FileGenerator_parameters(tmp_dataset):
    """Parameters are bound in the constructor and as attributes."""
    generator = ecole.instance.FileGenerator(directory=str(tmp_dataset), sampling_mode="remove")
    assert generator.sampling_mode.name == "remove"


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
