"""Test Ecole dynamics in Python.

Most dynamics classes are written in Ecole C++ library.
This is where the logic should be tested.
The tests provided here run the same assertions on all dynamics.
They mostly test that the code Bindings work as expected.
"""

import pytest
import numpy as np

import ecole


class DynamicsUnitTests:
    def test_default_init(self):
        """Construct with default arguments."""
        type(self.dynamics)()

    def test_reset(self, model):
        """Successive calls to reset."""
        self.dynamics.reset_dynamics(model.copy_orig())
        self.dynamics.reset_dynamics(model)

    def test_action_set(self, model):
        """The action set is of valid type."""
        _, action_set = self.dynamics.reset_dynamics(model)
        self.assert_action_set(action_set)

    def test_step(self, model):
        """Get an action_set and take an action."""
        done, action_set = self.dynamics.reset_dynamics(model)
        assert not done  # We need instances where solving is not trivial
        self.dynamics.step_dynamics(model, self.policy(action_set))

    @pytest.mark.slow
    def test_full_trajectory(self, model):
        """Run a complete trajectory."""
        done, action_set = self.dynamics.reset_dynamics(model)
        assert not done  # We need instances where solving is not trivial
        while not done:
            done, action_set = self.dynamics.step_dynamics(model, self.policy(action_set))

    def test_exception(self, model):
        """Bad action raise exceptions."""
        with pytest.raises((ecole.Exception, ecole.scip.Exception)):
            self.dynamics.reset_dynamics(model)
            self.dynamics.step_dynamics(model, self.bad_action)

    def test_set_random_state(self, model):
        """Random engine is consumed."""
        random_engine = ecole.RandomEngine(33)
        self.dynamics.set_dynamics_random_state(model, random_engine)
        assert random_engine != ecole.RandomEngine(33)


class TestBranching(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert isinstance(action_set, np.ndarray)
        assert action_set.ndim == 1
        assert action_set.size > 0
        assert action_set.dtype == np.uint64

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.BranchingDynamics(False)
        self.policy = lambda action_set: action_set[0]
        self.bad_action = 1 << 31


class TestBranchingPseudocost(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert isinstance(action_set, np.ndarray)
        assert action_set.ndim == 1
        assert action_set.size > 0
        assert action_set.dtype == np.uint64

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.BranchingDynamics(True)
        self.policy = lambda action_set: action_set[0]
        self.bad_action = 1 << 31


class TestConfiguring(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert action_set is None

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.ConfiguringDynamics()
        self.policy = lambda _: {
            "branching/scorefunc": "s",
            "branching/scorefac": 0.1,
            "branching/divingpscost": False,
            "conflict/lpiterations": 0,
            "heuristics/undercover/fixingalts": "ln",
        }
        self.bad_action = {"not/a/parameter": 44}
