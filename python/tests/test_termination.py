import ecole.termination as termination


def test_WhenSolved(model):
    assert termination.Constant(True).obtain_termination(model)
