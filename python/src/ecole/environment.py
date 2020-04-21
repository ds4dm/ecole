"""Ecole collection of environments."""

import ecole.core as core

from ecole.core.environment import *


class NoneFunction:
    """A State function to return None.

    An instance of this mock class return `None` on calls, and itself on
    attribute access.
    This is convenient to replace any State function, as any call such as
    ```
    self.reset_something(some, arguments)
    ```
    will return `None`.
    """

    @classmethod
    def as_default_of(cls, func):
        """Substitute the input with a new instance if the it is None."""
        if func is None:
            return cls()
        else:
            return func

    def __call__(self, *args, **kwargs):
        """Return None."""
        return None

    def __getattr__(self, attr):
        """Return itself."""
        return self


class EnvironmentComposer:

    __Dynamics__ = None
    __State__ = None

    def __init__(
        self,
        observation_function=None,
        reward_function=None,
        termination_function=None,
    ) -> None:
        self.observation_function = NoneFunction.as_default_of(observation_function)
        self.reward_function = NoneFunction.as_default_of(reward_function)
        self.termination_function = NoneFunction.as_default_of(termination_function)
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

            done = done or bool(self.termination_function.is_done(self.state))
            if done:
                observation = self.observation_function.get(self.state)
            else:
                observation = None
            return observation, done
        except Exception as e:
            self.can_transition = False
            raise e

    def step(self, action):
        if not self.can_transition:
            raise core.environment.Exception("Environment need to be reset.")

        try:
            done = self.dynamics.step_dynamics(self.state, action)
            done = done or bool(self.termination_function.is_done(self.state))
            reward = self.reward_function.get(self.state, done)
            if done:
                observation = self.observation_function.get(self.state)
            else:
                observation = None
            return observation, reward, done, {}
        except Exception as e:
            self.can_transition = False
            raise e


class Branching(EnvironmentComposer):
    __Dynamics__ = core.environment.BranchingDynamics
    __State__ = core.environment.ReverseControlState


class Configuring(EnvironmentComposer):
    __Dynamics__ = core.environment.ConfiguringDynamics
    __State__ = core.environment.State
