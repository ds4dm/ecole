"""Pytest configuration file."""

import pathlib

import pytest


TEST_SOURCE_DIR = pathlib.Path(__file__).parent.resolve()
DATA_DIR = TEST_SOURCE_DIR / "../../libecole/tests/data"


@pytest.fixture
def problem_file():
    """Return a MILP prblem file."""
    return DATA_DIR / "enlight8.mps"
