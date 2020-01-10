import ecole.termination as T


def test_Solved(model):
    termination_space = T.Solved()
    assert termination_space.is_done(model) is False
