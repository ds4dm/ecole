import pytest

from ecole.environment import Branching


@pytest.mark.slow
def test_branching_environment(model):
    env = Branching()
    for _ in range(2):
        count = 0
        obs, done = env.reset(model.clone())
        while not done:
            obs, reward, done, info = env.step(0)
            count += 1
        assert count > 0
