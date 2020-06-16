"""Typing information for Ecole.

Ecole flexibility relies on
`structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_
and therefore requires to explicit the structures at hand.
"""

from typing import Protocol, TypeVar

import ecole.scip


class RewardFunction(Protocol):
    """Class repsonsible for extracting rewards.

    Reward functions are objects given to the :py:class::`ecole.environment.EnvironmentComposer`
    to extract the reward used for learning.
    """

    def reset(self, model: ecole.scip.Model) -> None:
        """Reset internal data at the start of episodes.

        The method is called on new episodes
        :py:meth:`ecole.environment.EnvironmentComposer.reset` on the initial state.
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
        :py:meth:`ecole.environment.EnvironmentComposer.reset`, after :py:meth:`reset`, to obtain\
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
    :py:class::`ecole.environment.EnvironmentComposer` to extract the observations used to take the
    next action.
    """

    def reset(self, model: ecole.scip.Model) -> None:
        """Reset internal data at the start of episodes.

        The method is called on new episodes
        :py:meth:`ecole.environment.EnvironmentComposer.reset` on the initial state.
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
