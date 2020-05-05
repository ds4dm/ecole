import itertools

import pytest

import ecole.environment as environment


def product_parametrize(**params):
    names = ",".join(params.keys())
    values = itertools.product(*params.values())
    return pytest.mark.parametrize(names, values)


@product_parametrize(candidate=(0, -1), pseudo=(True, False))
def test_branching_environment_truncated(model, pseudo: bool, candidate: int):
    model.set_param("limits/totalnodes", 3)
    env = environment.Branching(pseudo_candidates=pseudo)
    for _ in range(2):
        count = 0
        obs, action_set, done = env.reset(model.clone())
        while not done:
            assert len(action_set) > 0
            obs, action_set, reward, done, info = env.step(action_set[candidate])
            count += 1
        assert count == 3


@pytest.mark.slow
def test_branching_environment_full(model):
    env = environment.Branching()
    for _ in range(2):
        count = 0
        obs, action_set, done = env.reset(model.clone())
        while not done:
            assert len(action_set) > 0
            obs, action_set, reward, done, info = env.step(action_set[0])
            count += 1
        assert count > 0


def test_branching_exception(model):
    with pytest.raises(environment.Exception):
        env = environment.Branching()
        env.step(-1)
