"""Pytest configuration file."""

import pathlib

import pytest

import ecole.scip


TEST_SOURCE_DIR = pathlib.Path(__file__).parent.resolve()
DATA_DIR = TEST_SOURCE_DIR / "../../libecole/tests/data"


@pytest.fixture
def problem_file():
    """Return a MILP prblem file."""
    return DATA_DIR / "enlight8.mps"


@pytest.fixture
def model(problem_file):
    """Return a Model object with a valid problem."""
    model_obj = ecole.scip.Model.from_file(str(problem_file))
    model_obj.disable_cuts()
    model_obj.disable_presolve()
    return model_obj
