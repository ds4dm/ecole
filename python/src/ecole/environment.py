"""Ecole collection of environments."""

import enum
from functools import partial

import ecole.core as core
import ecole.observation
import ecole.reward
import ecole.termination
from ecole.core.environment import *


class ConstantFunction:
    """A State function to return None.

    An instance of this mock class return the same value on calls, and itself
    on attribute access.
    This is convenient to replace any State function, as any call such as
    ```
    self.reset_something(some, arguments)
    ```
    will return the defined value.
    """

    def __init__(self, return_value):
        """Define the value to return."""
        self.return_value = return_value

    def __call__(self, *args, **kwargs):
        """Return None."""
        return self.return_value

    def __getattr__(self, attr):
        """Return itself."""
        return self


class EnvironmentComposer:

    __Dynamics__ = None
    __State__ = None
    __DefaultObservationFunction__ = partial(ConstantFunction, None)
    __DefaultRewardFunction__ = partial(ConstantFunction, 0)
    __DefaultTerminationFunction__ = partial(ConstantFunction, False)

    def __init__(
        self,
        observation_function="default",
        reward_function="default",
        termination_function="default",
    ) -> None:
        #  Set observation function
        if observation_function == "default":
            self.observation_function = self.__DefaultObservationFunction__()
        elif observation_function is None:
            self.observation_function = ConstantFunction(None)
        else:
            self.observation_function = observation_function

        #  Set reward function
        if reward_function == "default":
            self.reward_function = self.__DefaultRewardFunction__()
        elif reward_function is None:
            self.reward_function = ConstantFunction(0)
        else:
            self.reward_function = reward_function

        #  Set termination function
        if termination_function == "default":
            self.termination_function = self.__DefaultTerminationFunction__()
        elif termination_function is None:
            self.termination_function = ConstantFunction(False)
        else:
            self.termination_function = termination_function

        self.state = None
        self.dynamics = self.__Dynamics__()
        self.can_transition = False

    def reset(self, instance):
        self.can_transition = True
        try:
            if isinstance(instance, core.scip.Model):
                self.state = self.__State__(instance)
            else:
                self.state = self.__State__(core.scip.Model.from_file(instance))

            done = self.dynamics.reset_dynamics(self.state)
            self.termination_function.reset(self.state)
            self.observation_function.reset(self.state)
            self.reward_function.reset(self.state)

            done = done or self.termination_function.is_done(self.state)
            observation = self.observation_function.get(self.state)
            return observation, done
        except Exception as e:
            self.can_transition = False
            raise e

    def step(self, action):
        if not self.can_transition:
            raise core.environment.Exception("Environment need to be reset.")

        try:
            done = self.dynamics.step_dynamics(self.state, action)
            done = done or self.termination_function.is_done(self.state)
            reward = self.reward_function.get(self.state, done)
            observation = self.observation_function.get(self.state)
            return observation, reward, done, {}
        except Exception as e:
            self.can_transition = False
            raise e


class Branching(EnvironmentComposer):
    __Dynamics__ = core.environment.BranchingDynamics
    __State__ = core.environment.ReverseControlState
    __DefaultObservationFunction__ = ecole.observation.NodeBipartite
    __DefaultRewardFunction__ = ecole.reward.IsDone


class Configuring(EnvironmentComposer):
    __Dynamics__ = core.environment.ConfiguringDynamics
    __State__ = core.environment.State
    __DefaultRewardFunction__ = ecole.reward.IsDone
