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
    assert O.Nothing().get(state) is None


def test_NodeBipartite(solving_state):
    obs = O.NodeBipartite().get(solving_state)
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
