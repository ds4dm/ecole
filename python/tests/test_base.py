import ecole.base as B


def test_RewardSpace_Inheritance(model):
    class SpaceCalled(B.RewardSpace):
        def __init__(self):
            super().__init__()
            self.reset_called = False
            self.get_called = False

        def reset(self, *args, **kwargs):
            self.reset_called = True
            return super().reset(*args, **kwargs)

        def get(self, *args, **kwargs):
            self.get_called = True
            return 0

    space = SpaceCalled()
    space.reset(model)
    assert space.reset_called
    space.get(model)
    assert space.get_called


def test_Termination_Inheritance(model):
    class SpaceCalled(B.TerminationSpace):
        def __init__(self):
            super().__init__()
            self.reset_called = False
            self.get_called = False

        def reset(self, *args, **kwargs):
            self.reset_called = True
            return super().reset(*args, **kwargs)

        def is_done(self, *args, **kwargs):
            self.is_done_called = True
            return True

    space = SpaceCalled()
    space.reset(model)
    assert space.reset_called
    space.is_done(model)
    assert space.is_done_called
