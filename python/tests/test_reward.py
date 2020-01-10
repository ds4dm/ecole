import ecole.reward as R


def test_Done(model):
    reward_space = R.Done()
    reward_space.reset(model)
    assert reward_space.get(model) == 0
    assert reward_space.get(model, done=True) == 1
