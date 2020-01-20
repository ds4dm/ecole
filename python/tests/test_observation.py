import pytest

import ecole.observation as O


def test_BasicObsSpace(model):
    obs_space = O.BasicObsSpace()
    assert isinstance(obs_space, O.BasicObsSpace)
    obs_space.reset(model)
    obs = obs_space.get(model)
    assert isinstance(obs, O.BasicObs)


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
