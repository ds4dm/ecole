import unittest.mock as mock

import pytest
import numpy as np

import ecole.observation as O
from ecole.environment import Branching


@pytest.fixture
def solving_state(model):
    env = Branching()
    env.reset(model)
    return env.state


def test_Nothing(state):
    assert O.Nothing().obtain_observation(state) is None


def test_TupleFunction(state):
    """Dispach calls and pack the result in a tuple."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    tuple_obs_func = O.TupleFunction(obs_func1, obs_func2)

    tuple_obs_func.reset(state)
    obs_func1.reset.assert_called_once_with(state)
    obs_func2.reset.assert_called_once_with(state)

    obs_func1.obtain_observation.return_value = "something"
    obs_func2.obtain_observation.return_value = "else"
    obs = tuple_obs_func.obtain_observation(state)
    assert obs == ("something", "else")


def test_DictFunction(state):
    """Dispach calls and pack the result in a dict."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    dict_obs_func = O.DictFunction(name1=obs_func1, name2=obs_func2)

    dict_obs_func.reset(state)
    obs_func1.reset.assert_called_once_with(state)
    obs_func2.reset.assert_called_once_with(state)

    obs_func1.obtain_observation.return_value = "something"
    obs_func2.obtain_observation.return_value = "else"
    obs = dict_obs_func.obtain_observation(state)
    assert obs == {"name1": "something", "name2": "else"}


def test_NodeBipartite(solving_state):
    obs = O.NodeBipartite().obtain_observation(solving_state)
    assert isinstance(obs, O.NodeBipartiteObs)
    assert isinstance(obs.col_feat, np.ndarray)

    assert obs.col_feat.size > 0
    assert len(obs.col_feat.shape) == 2
    assert obs.row_feat.size > 0
    assert len(obs.row_feat.shape) == 2
    assert obs.matrix.shape == (obs.row_feat.shape[0], obs.col_feat.shape[0])
    assert obs.matrix.indices.shape == (2, obs.matrix.nnz)

    val = np.random.rand()
    obs.col_feat[:] = val
    assert np.all(obs.col_feat == val)
    obs.row_feat[:] = val
    assert np.all(obs.row_feat == val)
    obs.matrix.values[:] = val
    assert np.all(obs.matrix.values == val)
