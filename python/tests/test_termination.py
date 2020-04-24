import ecole.termination as termination


def test_WhenSolved(state):
    assert termination.Constant(True).obtain_termination(state)
