import ecole.reward as R


def test_Constant(state):
    assert R.Constant(33.0).get(state) == 33.0


def test_IsDone(state):
    reward_func = R.IsDone()
    reward_func.reset(state)
    assert reward_func.get(state) == 0
    assert reward_func.get(state, done=True) == 1


def test_NegLPIterations(state):
    reward_func = R.NegLPIterations()
    reward_func.reset(state)
    assert reward_func.get(state) <= 0
