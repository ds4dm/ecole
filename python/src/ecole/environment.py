"""Ecole collection of environments."""

import ecole.core as core
import ecole.observation
import ecole.reward
import ecole.termination
from ecole.core.environment import *


class EnvironmentComposer:

    __Dynamics__ = None
    __State__ = None
    __DefaultObservationFunction__ = ecole.observation.Nothing
    __DefaultRewardFunction__ = ecole.reward.IsDone
    __DefaultTerminationFunction__ = ecole.termination.Constant

    def __init__(
        self,
        observation_function="default",
        reward_function="default",
        termination_function="default",
        **dynamics_kwargs
    ) -> None:
        self.observation_function = self.__parse_observation_function(
            observation_function
        )

        #  Set reward function
        if reward_function == "default":
            self.reward_function = self.__DefaultRewardFunction__()
        elif reward_function is None:
            self.reward_function = ecole.reward.Constant(0.0)
        else:
            self.reward_function = reward_function

        #  Set termination function
        if termination_function == "default":
            self.termination_function = self.__DefaultTerminationFunction__()
        elif termination_function is None:
            self.termination_function = ecole.termination.Constant(False)
        else:
            self.termination_function = termination_function

        self.state = None
        self.dynamics = self.__Dynamics__(**dynamics_kwargs)
        self.can_transition = False

    @classmethod
    def __parse_observation_function(cls, observation_function):
        if observation_function == "default":
            return cls.__DefaultObservationFunction__()
        elif observation_function is None:
            return ecole.observation.Nothing()
        elif isinstance(observation_function, tuple):
            return ecole.observation.TupleFunction(
                *(cls.__parse_observation_function(fun) for fun in observation_function)
            )
        elif isinstance(observation_function, dict):
            return ecole.observation.DictFunction(
                **{
                    name: cls.__parse_observation_function(func)
                    for name, func in observation_function.items()
                }
            )
        else:
            return observation_function

    def reset(self, instance):
        """Reset environment to an initial state.

        This method brings the environment to a new initial state, *i.e.* starts a new
        episode.
        The method can be called at any point in time.

        Parameters
        ----------
        instance:
            The combinatorial optimization problem to tackle during the newly started
            episode.

        Returns
        -------
        observation:
            The observation of extracted from the initial state.
            Typically used to take the next action.
        action_set:
            An optional subset of accepted action in the next transition.
            For some environment, this may change at every transition.
        done:
            A boolean flag indicating wether the current state is terminal.
            If this is true, the episode is finished, and :meth:`step` cannot be called.

        """
        self.can_transition = True
        try:
            if isinstance(instance, core.scip.Model):
                self.state = self.__State__(instance)
            else:
                self.state = self.__State__(core.scip.Model.from_file(instance))

            done, action_set = self.dynamics.reset_dynamics(self.state)
            self.termination_function.reset(self.state)
            self.observation_function.reset(self.state)
            self.reward_function.reset(self.state)

            done = done or self.termination_function.obtain_termination(self.state)
            observation = self.observation_function.obtain_observation(self.state)
            return observation, action_set, done
        except Exception as e:
            self.can_transition = False
            raise e

    def step(self, action):
        """Transition from one state to another.

        This method takes a user action to transition from the current state to the
        next.
        The method **cannot** be called if the environment has not been reset since its
        instantiation or since a terminal state.

        Parameters
        ----------
        action:
            The action to take in as part of the Markov Decision Process.
            If an action set has been given in the latest call (inluding calls to
            :meth:`reset`), then the action **must** be in that set.

        Returns
        -------
        observation:
            The observation of extracted from the current state.
            Typically used to take the next action.
        action_set:
            An optional subset of accepted action in the next transition.
            For some environment, this may change at every transition.
        reward:
            A real number to use for reinforcement learning.
        done:
            A boolean flag indicating wether the current state is terminal.
            If this is true, the episode is finished, and this method cannot be called
            until :meth:`reset` has been called.
        info:
            A collection of environment specific information about the transition.
            This is not necessary for the control problem, but is useful to gain
            insights about the environment.

        """
        if not self.can_transition:
            raise core.environment.Exception("Environment need to be reset.")

        try:
            done, action_set = self.dynamics.step_dynamics(self.state, action)
            done = done or self.termination_function.obtain_termination(self.state)
            reward = self.reward_function.obtain_reward(self.state, done)
            observation = self.observation_function.obtain_observation(self.state)
            return observation, action_set, reward, done, {}
        except Exception as e:
            self.can_transition = False
            raise e

    def seed(self):
        """Set the random seed of the solver."""
        raise NotImplementedError()


class Branching(EnvironmentComposer):
    __Dynamics__ = core.environment.BranchingDynamics
    __State__ = core.environment.ReverseControlState
    __DefaultObservationFunction__ = ecole.observation.NodeBipartite


class Configuring(EnvironmentComposer):
    __Dynamics__ = core.environment.ConfiguringDynamics
    __State__ = core.environment.State
