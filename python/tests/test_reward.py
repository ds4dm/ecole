import math

import ecole.reward as R


def test_Constant(model):
    assert R.Constant(33.0).obtain_reward(model) == 33.0


def test_sub(model):
    func = R.Constant(4) - 3
    assert func.obtain_reward(model) == 1


def test_rsub(model):
    func = 4 - R.Constant(3)
    assert func.obtain_reward(model) == 1


def test_neg(model):
    func = -R.Constant(3)
    assert func.obtain_reward(model) == -3


def test_exp(model):
    func = R.Constant(3).exp()
    assert func.obtain_reward(model) == math.exp(3)


def test_recursive(model):
    func = abs(-R.Constant(2)) + 2
    assert func.obtain_reward(model) == 4


def test_apply(model):
    func = R.Constant(2).apply(lambda r: r + 2)
    assert func.obtain_reward(model) == 4


def test_IsDone(model):
    reward_func = R.IsDone()
    reward_func.reset(model)
    assert reward_func.obtain_reward(model) == 0
    assert reward_func.obtain_reward(model, done=True) == 1


def test_LpIterations(model):
    reward_func = R.LpIterations()
    reward_func.reset(model)
    assert reward_func.obtain_reward(model) <= 0
