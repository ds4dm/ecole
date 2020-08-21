"""Test Ecole observation functions in Python.

Most observation functions are written in Ecole C++ library.
This is where the logic should be tested.
Here,
  - Some tests automatically run the same assertions on all functions;
  - Other tests that observation returned form observation functions are bound to the correct types.
"""

import numpy as np

import ecole.observation as O


def pytest_generate_tests(metafunc):
    """Parametrize the `observation_function` fixture.

    Add observation functions here to have them automatically run all the tests that take
    `observation_function` as input.
    """
    if "observation_function" in metafunc.fixturenames:
        all_observation_functions = (
            O.Nothing(),
            O.NodeBipartite(),
            O.StrongBranchingScores(True),
            O.StrongBranchingScores(False),
            O.Pseudocosts(),
            O.Khalil2016(),
        )
        metafunc.parametrize("observation_function", all_observation_functions)


def test_default_init(observation_function):
    """Construct with default arguments."""
    type(observation_function)()


def test_before_reset(observation_function, solving_model):
    """Successive calls to before_reset."""
    observation_function.before_reset(solving_model)
    observation_function.before_reset(solving_model)


def test_extract(observation_function, solving_model):
    """Obtain observation."""
    observation_function.before_reset(solving_model)
    observation_function.extract(solving_model, False)


def make_obs(obs_func, model):
    obs_func.before_reset(model)
    return obs_func.extract(model, False)


def assert_array(arr, ndim=1, non_empty=True, dtype=np.double):
    assert isinstance(arr, np.ndarray)
    assert arr.ndim == ndim
    assert (not non_empty) or (arr.size > 0)
    assert arr.dtype == dtype


def test_Nothing_observation(model):
    """Observation of Nothing is None."""
    assert make_obs(O.Nothing(), model) is None


def test_NodeBipartite_observation(solving_model):
    """Observation of NodeBipartite is a type with array attributes."""
    obs = make_obs(O.NodeBipartite(), solving_model)
    assert isinstance(obs, O.NodeBipartiteObs)
    assert_array(obs.column_features, ndim=2)
    assert_array(obs.row_features, ndim=2)
    assert_array(obs.edge_features.values)
    assert_array(obs.edge_features.indices, ndim=2, dtype=np.uint64)


def test_StrongBranchingScores_observation(solving_model):
    """Observation of StrongBranchingScores is a numpy array."""
    obs = make_obs(O.StrongBranchingScores(), solving_model)
    assert_array(obs)


def test_Pseudocosts_observation(solving_model):
    """Observation of Pseudocosts is a numpy array."""
    obs = make_obs(O.Pseudocosts(), solving_model)
    assert_array(obs)


def test_Khalil2016_observatio(solving_model):
    """Observation of Khalil2016 is a numpy matrix."""
    obs = make_obs(O.Khalil2016(), solving_model)
    assert_array(obs, ndim=2)
