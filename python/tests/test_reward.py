import pytest

import ecole.reward as R


def test_Done(model):
    reward_space = R.Done()
    reward_space.reset(model)
    assert reward_space.get(model) == 0
    assert reward_space.get(model, done=True) == 1


@pytest.mark.parametrize("Class", (R.Done,))
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

    reward_space = SpaceCalled()
    reward_space.reset(model)
    assert reward_space.reset_called
    reward_space.get(model)
    assert reward_space.get_called
