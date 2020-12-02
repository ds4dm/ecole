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


def test_reset(model):
    """Reset with a model."""
    env = MockEnvironment()
    _, _, _, _, _ = env.reset(model)
    assert model is not env.model  # Model is copied
    assert not env.model.is_solved()

    env.dynamics.reset_dynamics.assert_called_with(env.model)
    env.dynamics.set_dynamics_random_state.assert_called()


def test_step(model):
    """Stepmwith some action."""
    env = MockEnvironment()
    env.reset(model)
    _, _, _, _, _ = env.step("some action")
    env.dynamics.step_dynamics.assert_called_with(env.model, "some action")


def test_seed():
    """Random engine is consumed."""
    env = MockEnvironment()
    env.seed(33)
    assert env.random_engine == ecole.RandomEngine(33)


def test_scip_params(model):
    """Reset sets parameters on the model."""
    env = MockEnvironment(scip_params={"concurrent/paramsetprefix": "testname"})
    env.reset(model)
    assert env.model.get_param("concurrent/paramsetprefix") == "testname"
