"""Test Ecole observation functions in Python.

Most observation functions are written in Ecole C++ library.
This is where the logic should be tested.
Here,
  - Some tests automatically run the same assertions on all functions;
  - Other tests that observation returned form observation functions are bound to the correct types.
"""

import unittest.mock as mock

import numpy as np

import ecole.observation as O


def test_TupleFunction(model):
    """Dispach calls and pack the result in a tuple."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    tuple_obs_func = O.TupleFunction(obs_func1, obs_func2)

    tuple_obs_func.reset(model)
    obs_func1.reset.assert_called_once_with(model)
    obs_func2.reset.assert_called_once_with(model)

    obs_func1.obtain_observation.return_value = "something"
    obs_func2.obtain_observation.return_value = "else"
    obs = tuple_obs_func.obtain_observation(model)
    assert obs == ("something", "else")


def test_DictFunction(model):
    """Dispach calls and pack the result in a dict."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    dict_obs_func = O.DictFunction(name1=obs_func1, name2=obs_func2)

    dict_obs_func.reset(model)
    obs_func1.reset.assert_called_once_with(model)
    obs_func2.reset.assert_called_once_with(model)

    obs_func1.obtain_observation.return_value = "something"
    obs_func2.obtain_observation.return_value = "else"
    obs = dict_obs_func.obtain_observation(model)
    assert obs == {"name1": "something", "name2": "else"}


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
        )
        metafunc.parametrize("observation_function", all_observation_functions)


def test_default_init(observation_function):
    """Construct with default arguments."""
    type(observation_function)()


def test_reset(observation_function, solving_model):
    """Successive calls to reset."""
    observation_function.reset(solving_model)
    observation_function.reset(solving_model)


def test_obtain_observation(observation_function, solving_model):
    """Obtain observation."""
    observation_function.reset(solving_model)
    observation_function.obtain_observation(solving_model)


def make_obs(obs_func, model):
    obs_func.reset(model)
    return obs_func.obtain_observation(model)


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


def test_Pseudocosts(solving_model):
    """Observation of Pseudocosts is a numpy array."""
    obs = make_obs(O.Pseudocosts(), solving_model)
    assert_array(obs)
