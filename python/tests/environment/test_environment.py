import unittest.mock as mock

import ecole


class MockDynamics(mock.MagicMock):
    def reset_dynamics(self, *args, **kwargs):
        return mock.MagicMock(), mock.MagicMock()


class MockEnvironment(ecole.environment.EnvironmentComposer):
    __Dynamics__ = MockDynamics


def test_observation_function_none(model):
    """None is parsed as no obseration."""
    env = MockEnvironment(observation_function=None)
    obs = env.observation_function.obtain_observation(model)
    assert obs is None


def test_observation_function_tuple(model):
    """Tuple are parsed as tuple of observations"""
    env = MockEnvironment(observation_function=(mock.MagicMock(), mock.MagicMock()))
    obs = env.observation_function.obtain_observation(model)
    assert isinstance(obs, tuple)
    assert len(obs) == 2


def test_observation_function_dict(model):
    """Dict are parsed as dict of observations"""
    obs_func = {"name1": mock.MagicMock(), "name2": mock.MagicMock()}
    env = MockEnvironment(observation_function=obs_func)
    obs = env.observation_function.obtain_observation(model)
    assert isinstance(obs, dict)
    assert len(obs) == 2
    assert ("name1" in obs) and ("name2" in obs)


def test_observation_function_recursive(model):
    """Parsing is recursive."""
    obs_func = {"name1": mock.MagicMock(), "name2": (mock.MagicMock(), None)}
    env = MockEnvironment(observation_function=obs_func)
    obs = env.observation_function.obtain_observation(model)
    assert obs["name2"][1] is None


def test_seed():
    env = MockEnvironment()
    env.seed(33)
    assert env.random_engine == ecole.environment.RandomEngine(33)


def test_scip_params(model):
    """Reset sets parameters on the model."""
    env = MockEnvironment(scip_params={"concurrent/paramsetprefix": "testname"})
    env.reset(model)
    assert model.get_param("concurrent/paramsetprefix") == "testname"
