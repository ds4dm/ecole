"""Pytest configuration file."""

import pathlib

import pytest

import ecole.scip
import ecole.environment


TEST_SOURCE_DIR = pathlib.Path(__file__).parent.resolve()
DATA_DIR = TEST_SOURCE_DIR / "../../libecole/tests/data"


def pytest_addoption(parser):
    """Add no-slow command line argument to pytest."""
    parser.addoption("--no-slow", action="store_true", default=False, help="do not run slow tests")


def pytest_configure(config):
    """Add slow marker to pytest."""
    config.addinivalue_line("markers", "slow: mark test as slow to run")


def pytest_collection_modifyitems(config, items):
    """Modify test collection to not run slow tests if not specified."""
    if config.getoption("--no-slow"):
        skip_slow = pytest.mark.skip(reason="--no-slow option provided")
        for item in items:
            if "slow" in item.keywords:
                item.add_marker(skip_slow)


@pytest.fixture
def problem_file():
    """Return a MILP problem file."""
    return DATA_DIR / "enlight8.mps"


@pytest.fixture
def model(problem_file):
    """Return a Model object with a valid problem."""
    model_obj = ecole.scip.Model.from_file(str(problem_file))
    model_obj.disable_cuts()
    model_obj.disable_presolve()
    return model_obj


@pytest.fixture
def model_copy(model):
    """Return a Model object with a valid problem."""
    return model.copy_orig()
