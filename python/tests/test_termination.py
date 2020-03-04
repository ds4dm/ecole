import pytest

import ecole.termination as T


def test_WhenSolved(state):
    termination_func = T.WhenSolved()
    termination_func.reset(state)
    assert termination_func.is_done(state) is False


@pytest.mark.parametrize("Class", (T.WhenSolved,))
def test_Inheritance(state, Class):
    class FunctionCalled(Class):
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

    term_func = FunctionCalled()
    term_func.reset(state)
    assert term_func.reset_called
    term_func.is_done(state)
    assert term_func.is_done_called
