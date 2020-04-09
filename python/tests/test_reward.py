import pytest

import ecole.reward as R


def test_IsDone(state):
    reward_func = R.IsDone()
    reward_func.reset(state)
    assert reward_func.get(state) == 0
    assert reward_func.get(state, done=True) == 1


def test_LPIterations(state):
    reward_func = R.LPIterations()
    reward_func.reset(state)
    assert reward_func.get(state) <= 0


@pytest.mark.parametrize("Class", (R.IsDone,))
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

    reward_func = FunctionCalled()
    reward_func.reset(state)
    assert reward_func.reset_called
    reward_func.get(state)
    assert reward_func.get_called
