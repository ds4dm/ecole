"""Ecole collection of environments."""

import ecole.core as core
import ecole.observation
import ecole.reward
from ecole.core.dynamics import *


class Environment:

    __Dynamics__ = None
    __DefaultObservationFunction__ = ecole.observation.Nothing
    __DefaultRewardFunction__ = ecole.reward.IsDone

    def __init__(
        self,
        observation_function="default",
        reward_function="default",
        scip_params=None,
        **dynamics_kwargs
    ) -> None:
        self.observation_function = self.__parse_observation_function(observation_function)
        self.reward_function = self.__parse_reward_function(reward_function)
        self.scip_params = scip_params if scip_params is not None else {}
        self.model = None
        self.dynamics = self.__Dynamics__(**dynamics_kwargs)
        self.can_transition = False
        self.random_engine = ecole.spawn_random_engine()

    @classmethod
    def __parse_reward_function(cls, reward_function):
        if reward_function == "default":
            return cls.__DefaultRewardFunction__()
        elif reward_function is None:
            return ecole.reward.Constant(0.0)
        else:
            return reward_function

    @classmethod
    def __parse_observation_function(cls, observation_function):
        if observation_function == "default":
            return cls.__DefaultObservationFunction__()
        elif observation_function is None:
            return ecole.observation.Nothing()
        elif isinstance(observation_function, tuple):
            return ecole.observation.VectorFunction(
                *(cls.__parse_observation_function(fun) for fun in observation_function)
            )
        elif isinstance(observation_function, dict):
            return ecole.observation.MapFunction(
                **{
                    name: cls.__parse_observation_function(func)
                    for name, func in observation_function.items()
                }
            )
        else:
            return observation_function

    def reset(self, instance, *dynamics_args, **dynamics_kwargs):
        """Start a new episode.

        This method brings the environment to a new initial state, *i.e.* starts a new
        episode.
        The method can be called at any point in time.

        Parameters
        ----------
        instance:
            The combinatorial optimization problem to tackle during the newly started
            episode.
        dynamics_args:
            Extra arguments are forwarded as is to the underlying :py:class:`~ecole.typing.Dynamics`.
        dynamics_kwargs:
            Extra arguments are forwarded as is to the underlying :py:class:`~ecole.typing.Dynamics`.

        Returns
        -------
        observation:
            The observation of extracted from the initial state.
            Typically used to take the next action.
        action_set:
            An optional subset of accepted action in the next transition.
            For some environment, this may change at every transition.
        reward_offset:
            An offset on the initial state.
            This reward is not used for learning (as no action has yet been taken) but is used in
            evaluation for the sum of rewards when one needs to account for computations that
            happened during :py:meth:`reset` (*e.g.* computation time, number of LP iteration in
            presolving...).
        done:
            A boolean flag indicating wether the current state is terminal.
            If this is true, the episode is finished, and :meth:`step` cannot be called.

        """
        self.can_transition = True
        try:
            if isinstance(instance, core.scip.Model):
                self.model = instance
            else:
                self.model = core.scip.Model.from_file(instance)
            self.model.set_params(self.scip_params)

            self.dynamics.set_dynamics_random_state(self.model, self.random_engine)

            done, action_set = self.dynamics.reset_dynamics(
                self.model, *dynamics_args, **dynamics_kwargs
            )
            self.observation_function.reset(self.model)
            self.reward_function.reset(self.model)

            reward_offset = self.reward_function.extract(self.model, done)
            observation = self.observation_function.obtain_observation(self.model)
            return observation, action_set, reward_offset, done
        except Exception as e:
            self.can_transition = False
            raise e

    def step(self, action, *dynamics_args, **dynamics_kwargs):
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
        dynamics_args:
            Extra arguments are forwarded as is to the underlying :py:class:`~ecole.typing.Dynamics`.
        dynamics_kwargs:
            Extra arguments are forwarded as is to the underlying :py:class:`~ecole.typing.Dynamics`.

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
            done, action_set = self.dynamics.step_dynamics(
                self.model, action, *dynamics_args, **dynamics_kwargs
            )
            reward = self.reward_function.extract(self.model, done)
            observation = self.observation_function.obtain_observation(self.model)
            return observation, action_set, reward, done, {}
        except Exception as e:
            self.can_transition = False
            raise e

    def seed(self, value: int) -> None:
        """Set the random seed of the environment.

        The the random seed is used to seed the environment :py:class:`~ecole.RandomEngine`.
        At every call to :py:meth:`reset`, the random engine is used to create new seeds
        for the solver.
        Setting the seed once will ensure determinism for the next trajectories.
        By default, the random engine is initialized by the
        `random <https://docs.python.org/library/random.html>`_ module.
        """
        self.random_engine.seed(value)


class Branching(Environment):
    __Dynamics__ = core.dynamics.BranchingDynamics
    __DefaultObservationFunction__ = ecole.observation.NodeBipartite


class Configuring(Environment):
    __Dynamics__ = core.dynamics.ConfiguringDynamics
