import ecole.termination as T


def test_Solved(model):
    termination_space = T.Solved()
    termination_space.reset(model)
    assert termination_space.is_done(model) is False
