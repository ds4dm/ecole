import pytest

import ecole.termination as T


def test_Solved(model):
    termination_space = T.Solved()
    termination_space.reset(model)
    assert termination_space.is_done(model) is False


@pytest.mark.parametrize("Class", (T.Solved,))
def test_Inheritance(model, Class):
    class SpaceCalled(Class):
        def __init__(self):
            super().__init__()
            self.reset_called = False
            self.get_called = False

        def reset(self, *args, **kwargs):
            self.reset_called = True
            return super().reset(*args, **kwargs)

        def is_done(self, *args, **kwargs):
            self.is_done_called = True
            return super().is_done(*args, **kwargs)

    term_space = SpaceCalled()
    term_space.reset(model)
    assert term_space.reset_called
    term_space.is_done(model)
    assert term_space.is_done_called
