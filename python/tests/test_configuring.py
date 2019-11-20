import pytest

import ecole.configuring
import ecole.scip


@pytest.fixture
def model(problem_file):
    return ecole.scip.Model.from_file(str(problem_file))


def test_Configure(model):
    conf = ecole.configuring.Configure("conflict/lpiterations")
    conf.set(model, 0)


def test_Configure_char(model):
    conf = ecole.configuring.Configure("branching/scorefunc")
    conf.set(model, "s")


def test_Configure_error(model):
    conf = ecole.configuring.Configure("conflict/lpiterations")
    # FIXME test with scip.Exception
    with pytest.raises(Exception):
        conf.set(model, [1, 2])


def test_configuring_env(model):
    env = ecole.configuring.Env.make_dummy("conflict/lpiterations")
    for _ in range(2):
        count = 0
        obs, done = env.reset(model)
        while not done:
            obs, reward, done, info = env.step(0)
            count += 1
        assert count == 1
