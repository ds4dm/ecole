import pytest
import numpy as np

import ecole.observation as O


def test_NodeBipartite(state):
    obs_func = O.NodeBipartite()
    assert isinstance(obs_func, O.NodeBipartite)
    obs_func.reset(state)
    obs = obs_func.get(state)
    assert isinstance(obs, O.NodeBipartiteObs)


def test_NodeBipartite(state):
    obs = O.NodeBipartite().get(state)
    assert isinstance(obs.col_feat, np.ndarray)

    assert obs.col_feat.size > 0
    old_col_feat = np.copy(obs.col_feat)
    # A transformation that leaves all elements changed (even with float approx).
    obs.col_feat[:] = 2 * obs.col_feat + 1
    assert np.all(obs.col_feat != old_col_feat)


@pytest.mark.parametrize("Class", (O.NodeBipartite,))
def test_Inheritance(state, Class):
    class FunctionCalled(Class):
        def __init__(self):
            super().__init__()
            self.reset_called = False
            self.get_called = False

        def reset(self, *args, **kwargs):
            self.reset_called = True
            return super().reset(*args, **kwargs)

        def get(self, *args, **kwargs):
            self.get_called = True
            return super().get(*args, **kwargs)

    function = FunctionCalled()
    function.reset(state)
    assert function.reset_called
    function.get(state)
    assert function.get_called
