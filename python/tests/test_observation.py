import pytest
import numpy as np

import ecole.observation as O


def test_BasicObsSpace(model):
    obs_space = O.BasicObsSpace()
    assert isinstance(obs_space, O.BasicObsSpace)
    obs_space.reset(model)
    obs = obs_space.get(model)
    assert isinstance(obs, O.BasicObs)


def test_BasicObs(model):
    obs = O.BasicObsSpace().get(model)
    assert isinstance(obs.var_feat, np.ndarray)

    assert obs.var_feat.size > 0
    old_var_feat = np.copy(obs.var_feat)
    # A transformation that leaves all elements changed (even with float approx).
    obs.var_feat[:] = 2 * obs.var_feat + 1
    assert np.all(obs.var_feat != old_var_feat)


@pytest.mark.parametrize("Class", (O.BasicObsSpace,))
def test_Inheritance(model, Class):
    class SpaceCalled(Class):
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

    space = SpaceCalled()
    space.reset(model)
    assert space.reset_called
    space.get(model)
    assert space.get_called
