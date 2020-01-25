import pytest
import numpy as np

import ecole.observation as O


def test_BasicObsFunction(model):
    obs_func = O.BasicObsFunction()
    assert isinstance(obs_func, O.BasicObsFunction)
    obs_func.reset(model)
    obs = obs_func.get(model)
    assert isinstance(obs, O.BasicObs)


def test_BasicObs(model):
    obs = O.BasicObsFunction().get(model)
    assert isinstance(obs.var_feat, np.ndarray)

    assert obs.var_feat.size > 0
    old_var_feat = np.copy(obs.var_feat)
    # A transformation that leaves all elements changed (even with float approx).
    obs.var_feat[:] = 2 * obs.var_feat + 1
    assert np.all(obs.var_feat != old_var_feat)


@pytest.mark.parametrize("Class", (O.BasicObsFunction,))
def test_Inheritance(model, Class):
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
    function.reset(model)
    assert function.reset_called
    function.get(model)
    assert function.get_called
