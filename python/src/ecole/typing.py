"""Typing information for Ecole.

Ecole flexibility relies on
`structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_
and therefore requires to explicit the structures at hand.
"""

from typing import TypeVar, Tuple

try:
    from typing import Protocol
except ImportError:
    from typing_extensions import Protocol

import ecole


Action = TypeVar("Action")
ActionSet = TypeVar("ActionSet")


class Dynamics(Protocol[Action, ActionSet]):
    """Dynamics are raw environments.

    The class is a bare :py:class:`ecole.environment.EnvironmentComposer` without rewards,
    observations, and other utlilities.
    It defines the state transitions of a Markov Decision Process, that is the series of steps and
    possible actions of the environment.
    """

    def set_dynamics_random_state(
        self, model: ecole.scip.Model, random_engine: ecole.RandomEngine
    ) -> None:
        """Set the random state of the episode.

        This method is called by :py:meth:`~ecole.environment.EnvironmentComposer.reset` to
        set all the random elements of the dynamics for the upcoming episode.
        The random engine is kept between episodes in order to sample different episodes.

        Parameters
        ----------
        model:
            The SCIP model that will be used through the episode.
        random_engine:
            The random engine used by the environment from which random numbers can be extracted.

        """
        ...

    def reset_dynamics(self, model: ecole.scip.Model) -> Tuple[bool, ActionSet]:
        """Start a new episode.

        This method brings the environment to a new initial state, *i.e.* starts a new
        episode.
        The method can be called at any point in time.

        Parameters
        ----------
        model:
            The SCIP model that will be used through the episode.

        Returns
        -------
        done:
            A boolean flag indicating wether the current state is terminal.
            If this is true, the episode is finished, and :meth:`step_dynamics` cannot be called.
        action_set:
            An optional subset of accepted action in the next transition.
            For some environment, this may change at every transition.

        """
        ...

    def step_dynamics(self, model: ecole.scip.Model, action: Action) -> Tuple[bool, ActionSet]:
        """Transition from one state to another.

        This method takes the user action to transition from the current state to the
        next.
        The method **cannot** be called if the dynamics has not been reset since its
        instantiation or is in a terminal state.

        Parameters
        ----------
        action:
            The action to take in as part of the Markov Decision Process.
            If an action set has been given in the latest call (inluding calls to
            :meth:`reset_dynamics`), then the action **must** be in that set.

        Returns
        -------
        done:
            A boolean flag indicating wether the current state is terminal.
            If this is true, the episode is finished, and this method cannot be called
            until :meth:`reset_dynamics` has been called.
        action_set:
            An optional subset of accepted action in the next transition.
            For some environment, this may change at every transition.

        """
        ...


class RewardFunction(Protocol):
    """Class repsonsible for extracting rewards.

    Reward functions are objects given to the :py:class:`~ecole.environment.EnvironmentComposer`
    to extract the reward used for learning.
    """

    def reset(self, model: ecole.scip.Model) -> None:
        """Reset internal data at the start of episodes.

        The method is called on new episodes
        :py:meth:`~ecole.environment.EnvironmentComposer.reset` on the initial state.
        It can is usually used to reset the observation function internal data.

        Parameters
        ----------
        model:
            The SCIP model defining the current state of the solver.

        """
        ...

    def obtain_reward(self, model: ecole.scip.Model, done: bool) -> float:
        """Extract reward for arriving on given state.

        Extract the reward for arriving on the state given by ``model``.
        A reward is typically computed by transitioning from a state ``S1`` to a state ``S2``.
        For perfomance reasons, intermediate states are not kept.
        The reward function is reponsible for keeping track of relevant information from previous
        states.
        This can safely be done in this method as it will only be called *once per state*
        *i.e.*, this method is not a getter and can have side effects.

        Note that the method is also called on
        :py:meth:`~ecole.environment.EnvironmentComposer.reset`, after :py:meth:`reset`, to obtain
        the ``reward_offset``.

        Parameters
        ----------
        model:
            The SCIP model defining the current state of the solver.
        done:
            A flag indicating wether the state is terminal (as decided by the environment).

        Returns
        -------
        :
            The return is passed to the user by the environment.

        """
        ...


Observation = TypeVar("Observation")


class ObservationFunction(Protocol[Observation]):
    """Class repsonsible for extracting observations.

    Observation functions are objects given to the
    :py:class:`~ecole.environment.EnvironmentComposer` to extract the observations used to take the
    next action.
    """

    def reset(self, model: ecole.scip.Model) -> None:
        """Reset internal data at the start of episodes.

        The method is called on new episodes
        :py:meth:`~ecole.environment.EnvironmentComposer.reset` on the initial state.
        It can is usually used to reset the observation function internal data.

        Parameters
        ----------
        model:
            The SCIP model defining the current state of the solver.

        """
        ...

    def obtain_observation(self, model: ecole.scip.Model) -> Observation:
        """Extract observation on a given state.

        The observation describe the state and is used for learning.
        This method will only be called *once per state* *i.e.*, this method is not a getter and
        can have side effects.

        Parameters
        ----------
        model:
            The SCIP model defining the current state of the solver.

        Returns
        -------
        :
            The return is passed to the user by the environment.

        """
        ...
