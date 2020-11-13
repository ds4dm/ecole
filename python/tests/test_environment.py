"""Unit tests for Ecole Environment."""

import unittest.mock as mock

import ecole


class MockDynamics(mock.MagicMock):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.reset_dynamics = mock.MagicMock(return_value=(False, "some_action_set"))
        self.step_dynamics = mock.MagicMock(return_value=(True, "other_action_set"))


class MockEnvironment(ecole.environment.Environment):
    __Dynamics__ = MockDynamics


def test_observation_function_none(model):
    """None is parsed as no observation."""
    env = MockEnvironment(observation_function=None)
    obs = env.observation_function.extract(model, False)
    assert obs is None


def test_observation_function_vector(model):
    """Tuple are parsed as tuple of observations"""
    env = MockEnvironment(observation_function=(mock.MagicMock(), mock.MagicMock()))
    obs = env.observation_function.extract(model, False)
    assert isinstance(obs, list)
    assert len(obs) == 2


def test_observation_function_dict(model):
    """Dict are parsed as dict of observations"""
    obs_func = {"name1": mock.MagicMock(), "name2": mock.MagicMock()}
    env = MockEnvironment(observation_function=obs_func)
    obs = env.observation_function.extract(model, False)
    assert isinstance(obs, dict)
    assert len(obs) == 2
    assert ("name1" in obs) and ("name2" in obs)


def test_observation_function_recursive(model):
    """Parsing is recursive."""
    obs_func = {"name1": mock.MagicMock(), "name2": (mock.MagicMock(), None)}
    env = MockEnvironment(observation_function=obs_func)
    obs = env.observation_function.extract(model, False)
    assert obs["name2"][1] is None


def test_reset(model):
    """Reset with a model."""
    env = MockEnvironment()
    _, _, _, _ = env.reset(model)
    env.dynamics.reset_dynamics.assert_called_with(model)
    env.dynamics.set_dynamics_random_state.assert_called()


def test_step(model):
    """Stepmwith some action."""
    env = MockEnvironment()
    env.reset(model)
    _, _, _, _, _ = env.step("some action")
    env.dynamics.step_dynamics.assert_called_with(model, "some action")


def test_seed():
    """Random engine is consumed."""
    env = MockEnvironment()
    env.seed(33)
    assert env.random_engine == ecole.RandomEngine(33)


def test_scip_params(model):
    """Reset sets parameters on the model."""
    env = MockEnvironment(scip_params={"concurrent/paramsetprefix": "testname"})
    env.reset(model)
    assert model.get_param("concurrent/paramsetprefix") == "testname"
