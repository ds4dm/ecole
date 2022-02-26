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
        with pytest.raises((ecole.scip.ScipError, ValueError)):
            _, action_set = self.dynamics.reset_dynamics(model)
            self.dynamics.step_dynamics(model, self.bad_policy(action_set))

    def test_set_random_state(self, model):
        """Random generator is consumed."""
        rng = ecole.RandomGenerator(33)
        self.dynamics.set_dynamics_random_state(model, rng)
        assert rng != ecole.RandomGenerator(33)


class TestBranching(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert isinstance(action_set, np.ndarray)
        assert action_set.ndim == 1
        assert action_set.size > 0
        assert action_set.dtype == np.uint64

    @staticmethod
    def policy(action_set):
        return action_set[0]

    @staticmethod
    def bad_policy(action_set):
        return 1 << 31

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.BranchingDynamics(False)


class TestBranchingDefault(TestBranching):
    @staticmethod
    def policy(action_set):
        return ecole.Default


class TestBranching_Pseudocandidate(TestBranching):
    def setup_method(self, method):
        self.dynamics = ecole.dynamics.BranchingDynamics(True)


class TestBranchingSum_List(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert isinstance(action_set, np.ndarray)
        assert action_set.ndim == 1
        assert action_set.size > 0
        assert action_set.dtype == np.uint64

    @staticmethod
    def policy(action_set):
        return [action_set[0]]

    @staticmethod
    def bad_policy(action_set):
        return [1 << 31]

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.BranchingSumDynamics()


class TestBranchingSum_Numpy(TestBranchingSum_List):
    @staticmethod
    def policy(action_set):
        return np.array([action_set[0]])


class TestConfiguring(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert action_set is None

    @staticmethod
    def policy(action_set):
        return {
            "branching/scorefunc": "s",
            "branching/scorefac": 0.1,
            "branching/divingpscost": False,
            "conflict/lpiterations": 0,
            "heuristics/undercover/fixingalts": "ln",
        }

    @staticmethod
    def bad_policy(action_set):
        return {"not/a/parameter": 44}

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.ConfiguringDynamics()


class TestPrimalSearch(DynamicsUnitTests):
    @staticmethod
    def assert_action_set(action_set):
        assert isinstance(action_set, np.ndarray)
        assert action_set.ndim == 1
        assert action_set.size > 0
        assert action_set.dtype == np.uint64

    @staticmethod
    def policy(action_set):
        # Mixed numpy array and list
        return (action_set, [0.0] * len(action_set))

    @staticmethod
    def bad_policy(action_set):
        return ([1 << 31], [0.0])

    def setup_method(self, method):
        self.dynamics = ecole.dynamics.PrimalSearchDynamics()
