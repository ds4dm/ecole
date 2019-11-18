import pytest

import ecole.branching
import ecole.scip


@pytest.fixture
def model(problem_file):
    model = ecole.scip.Model.from_file(str(problem_file))
    model.disable_cuts()
    model.disable_presolve()
    return model


def test_branching_env(model):
    env = ecole.branching.Env.make_dummy()
    for _ in range(2):
        count = 0
        obs, done = env.reset(model)
        while not done:
            obs, reward, done, info = env.step(0)
            count += 1
        assert count > 0
