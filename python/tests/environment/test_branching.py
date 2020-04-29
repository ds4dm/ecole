import pytest

import ecole.environment as environment


@pytest.mark.slow
def test_branching_environment(model):
    env = environment.Branching()
    for _ in range(2):
        count = 0
        obs, action_set, done = env.reset(model.clone())
        while not done:
            obs, action_set, reward, done, info = env.step(action_set[0])
            count += 1
        assert count > 0


def test_exception(model):
    with pytest.raises(environment.Exception):
        env = environment.Branching()
        env.step(0)
