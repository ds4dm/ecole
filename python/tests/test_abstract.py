import ecole.abstract as abst


def test_ObservationFunction_Inheritance(model):
    class FunctionCalled(abst.ObservationFunction):
        def __init__(self):
            super().__init__()
            self.reset_called = False
            self.get_called = False

        def reset(self, *args, **kwargs):
            self.reset_called = True
            return super().reset(*args, **kwargs)

        def get(self, *args, **kwargs):
            self.get_called = True
            return abst.Observation()

    function = FunctionCalled()
    function.reset(model)
    assert function.reset_called
    function.get(model)
    assert function.get_called


def test_RewardFunction_Inheritance(model):
    class FunctionCalled(abst.RewardFunction):
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

    function = FunctionCalled()
    function.reset(model)
    assert function.reset_called
    function.get(model)
    assert function.get_called


def test_Termination_Inheritance(model):
    class FunctionCalled(abst.TerminationFunction):
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

    function = FunctionCalled()
    function.reset(model)
    assert function.reset_called
    function.is_done(model)
    assert function.is_done_called
