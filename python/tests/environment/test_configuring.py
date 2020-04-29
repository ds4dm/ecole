import pytest

from ecole.environment import Configuring


@pytest.mark.slow
def test_configuring_environment(model):
    action = {
        "branching/scorefunc": "s",
        "branching/scorefac": 0.1,
        "branching/divingpscost": False,
        "conflict/lpiterations": 0,
        "heuristics/undercover/fixingalts": "ln",
    }

    env = Configuring()
    for _ in range(2):
        count = 0

        obs, _, done = env.reset(model.clone())
        while not done:
            obs, _, reward, done, info = env.step(action)
            count += 1

        assert count == 1
        for param, value in action.items():
            assert env.state.model.get_param(param) == value
