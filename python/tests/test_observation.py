import unittest.mock as mock

import numpy as np

import ecole.observation as O
from ecole.environment import Branching


def test_Nothing(model):
    assert O.Nothing().obtain_observation(model) is None


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


def test_NodeBipartite(solving_model):
    obs = O.NodeBipartite().obtain_observation(solving_model)
    assert isinstance(obs, O.NodeBipartiteObs)
    assert isinstance(obs.column_features, np.ndarray)

    assert obs.column_features.size > 0
    assert len(obs.column_features.shape) == 2
    assert obs.row_features.size > 0
    assert len(obs.row_features.shape) == 2
    assert obs.edge_features.shape == (obs.row_features.shape[0], obs.column_features.shape[0],)
    assert obs.edge_features.indices.shape == (2, obs.edge_features.nnz)

    val = np.random.rand()
    obs.column_features[:] = val
    assert np.all(obs.column_features == val)
    obs.row_features[:] = val
    assert np.all(obs.row_features == val)
    obs.edge_features.values[:] = val
    assert np.all(obs.edge_features.values == val)


def test_StrongBranchingScores(solving_model):
    obs = O.StrongBranchingScores().obtain_observation(solving_model)
    assert isinstance(obs, np.ndarray)
    assert obs.size > 0
    assert len(obs.shape) == 1
