import pytest

import ecole.environment as environment


@pytest.mark.parametrize("candidate", (0, -1))
def test_branching_environment_truncated(model, candidate: int):
    model.set_param("limits/totalnodes", 5)
    env = environment.Branching()
    for _ in range(2):
        count = 0
        obs, action_set, done = env.reset(model.clone())
        while not done:
            assert len(action_set) > 0
            obs, action_set, reward, done, info = env.step(action_set[candidate])
            count += 1
        assert count == 5


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
