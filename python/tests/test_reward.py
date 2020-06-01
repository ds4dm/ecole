import math

import ecole.reward as R


def test_Constant(state):
    assert R.Constant(33.0).obtain_reward(state) == 33.0


def test_sub(state):
    func = R.Constant(4) - 3
    assert func.obtain_reward(state) == 1


def test_rsub(state):
    func = 4 - R.Constant(3)
    assert func.obtain_reward(state) == 1


def test_neg(state):
    func = -R.Constant(3)
    assert func.obtain_reward(state) == -3


def test_exp(state):
    func = R.Constant(3).exp()
    assert func.obtain_reward(state) == math.exp(3)


def test_recursive(state):
    func = abs(-R.Constant(2)) + 2
    assert func.obtain_reward(state) == 4


def test_apply(state):
    func = R.Constant(2).apply(lambda r: r + 2)
    assert func.obtain_reward(state) == 4


def test_IsDone(state):
    reward_func = R.IsDone()
    reward_func.reset(state)
    assert reward_func.obtain_reward(state) == 0
    assert reward_func.obtain_reward(state, done=True) == 1


def test_LpIterations(state):
    reward_func = R.LpIterations()
    reward_func.reset(state)
    assert reward_func.obtain_reward(state) <= 0
