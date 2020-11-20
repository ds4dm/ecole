"""Typing information for Ecole.

Ecole flexibility relies on
`structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_
and therefore requires to explicit the structures at hand.
"""

from typing import TypeVar, Tuple, Dict, Iterator, Any, overload

try:
    from typing import Protocol
except ImportError:
    from typing_extensions import Protocol

import ecole


Action = TypeVar("Action")
ActionSet = TypeVar("ActionSet")


class Dynamics(Protocol[Action, ActionSet]):
    """Dynamics are raw environments.

    The class is a bare :py:class:`ecole.environment.Environment` without rewards,
    observations, and other utlilities.
    It defines the state transitions of a Markov Decision Process, that is the series of steps and
    possible actions of the environment.
    """

    def set_dynamics_random_state(
        self, model: ecole.scip.Model, random_engine: ecole.RandomEngine
    ) -> None:
        """Set the random state of the episode.

        This method is called by :py:meth:`~ecole.environment.Environment.reset` to
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


Data = TypeVar("Data")


class DataFunction(Protocol[Data]):
    """The parent class of all function extracting data from the environment.

    Data functions are a generic alias for :py:class:`~ecole.typing.ObservationFunction`,
    :py:class:`~ecole.typing.RewardFunction`, and :py:class:`~ecole.typing.InformationFunction`
    with different data types, such as float for rewards.

    Having a similar interface between them makes it easier to combine them in various ways, such
    as creating :py:class:`~ecole.typing.ObservationFunction` or :py:class:`~ecole.typing.InformationFunction`
    from a dictionnary of :py:class:`~ecole.typing.RewardFunction`.

    This class is meant to represent a function of the whole state trajectory/history.
    However, because it is not feasible to keep all the previous states in memory, this equivalent
    implementation as a class let the object store information from one transition to another.

    See Also
    --------
    RewardFunction

    """

    def before_reset(self, model: ecole.scip.Model) -> None:
        """Reset internal data at the start of episodes.

        The method is called on new episodes :py:meth:`~ecole.environment.Environment.reset` right before
        the MDP is actually reset, that is right before the environment calls
        :py:meth:`~ecole.typing.Dynamics.reset_dynamics`.

        It is usually used to reset the internal data.

        Parameters
        ----------
        model:
            The :py:class:`~ecole.scip.Model`, model defining the current state of the solver.

        """
        ...

    def extract(self, model: ecole.scip.Model, done: bool) -> Data:
        """Extract the data on the given state.

        Extract the data  after transitionning on the new state given by ``model``.
        The function is reponsible for keeping track of relevant information from previous states.
        This can safely be done in this method as it will only be called *once per state* *i.e.*,
        this method is not a getter and can have side effects.

        Parameters
        ----------
        model:
            The :py:class:`~ecole.scip.Model`, model defining the current state of the solver.
        done:
            A flag indicating wether the state is terminal (as decided by the environment).

        Returns
        -------
        :
            The return is passed to the user by the environment.

        """
        ...


def _set_docstring(doc):
    """Decorator to dynamically set docstring."""

    def decorator(func):
        func.__doc__ = doc
        return func

    return decorator


Observation = TypeVar("Observation")


class ObservationFunction(DataFunction[Observation], Protocol[Observation]):
    """Class repsonsible for extracting observations.

    Observation functions are objects given to the :py:class:`~ecole.environment.Environment` to
    extract the observations used to take the next action.

    This class presents the interface expected to define a valid observation function.
    It is not necessary to inherit from this class, as observation functions are defined by
    `structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_.
    It is exists to support Python type hints.

    See Also
    --------
    DataFunction :
        Observation function are equivalent to the generic data function, that is a function to
        extact an arbitrary type of data.

    """

    @_set_docstring(DataFunction.before_reset.__doc__)
    def before_reset(self, model: ecole.scip.Model) -> None:
        ...

    @_set_docstring(DataFunction.extract.__doc__.replace("data", "observation"))
    def extract(self, model: ecole.scip.Model, done: bool) -> Observation:
        ...


class RewardFunction(DataFunction[float], Protocol):
    """Class responsible for extracting rewards.

    Reward functions are objects given to the :py:class:`~ecole.environment.Environment`
    to extract the reward used for learning.

    This class presents the interface expected to define a valid reward function.
    It is not necessary to inherit from this class, as reward functions are defined by
    `structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_.
    It is exists to support Python type hints.

    Note
    ----
    Rewards, or rather reward offset, are also extracted on :py:meth:`~ecole.environment.Environment.reset`.
    This has no use for learning (since not action has been taken), but is useful when using the cumulative
    reward sum as a metric.

    See Also
    --------
    DataFunction :
        Reward function are a specific type of generic data function where the data extracted are reward
        of type ``float``.

    """

    @_set_docstring(DataFunction.before_reset.__doc__)
    def before_reset(self, model: ecole.scip.Model) -> None:
        ...

    @_set_docstring(DataFunction.extract.__doc__.replace("data", "reward"))
    def extract(self, model: ecole.scip.Model, done: bool) -> float:
        ...


Information = TypeVar("Information")


class InformationFunction(DataFunction[Dict[str, Information]], Protocol[Information]):
    """Class repsonsible for extracting the the information dictionnary.

    Information functions are objects given to the :py:class:`~ecole.environment.Environment` to
    extract the addtional information about the environment.

    A common pattern is use additional :py:class:`ecole.typing.RewardFunction` and
    :py:class:`ecole.typing.ObservationFunction` to easily create information functions.

    This class presents the interface expected to define a valid information function.
    It is not necessary to inherit from this class, as information functions are defined by
    `structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_.
    It is exists to support Python type hints.

    See Also
    --------
    DataFunction :
        Information function are a specific type of generic data function where the data extracted
        are dictionnary of string to any type.

    """

    @_set_docstring(DataFunction.before_reset.__doc__)
    def before_reset(self, model: ecole.scip.Model) -> None:
        ...

    @_set_docstring(DataFunction.extract.__doc__.replace("data", "information"))
    def extract(self, model: ecole.scip.Model, done: bool) -> Dict[str, Information]:
        ...


class InstanceGenerator(Protocol):
    """A class to generate generate and iteratate over random problem instance.

    The class combines a :py:class:`~ecole.RandomEngine` with the static function :py:meth:`generate_instance`
    to provide iterating capabilities.
    """

    @staticmethod
    def generate_instance(
        *args: Any, random_engine: ecole.RandomEngine, **kwargs: Any
    ) -> ecole.scip.Model:
        """Generate a problem instance using the random engine for any source of randomness."""
        ...

    @overload
    def __init__(self, *args: Any, random_engine: ecole.RandomEngine, **kwargs: Any) -> None:
        """Create an iterator with the given parameters and a copy of the random state."""
        ...

    def __next__(self) -> ecole.scip.Model:
        """Generate a problem instance using the random engine of the class."""
        ...

    def __iter__(self) -> Iterator[ecole.scip.Model]:
        """Return itself as an iterator."""
        ...

    def seed(self, int) -> None:
        """Seed the random engine of the class."""
        ...
