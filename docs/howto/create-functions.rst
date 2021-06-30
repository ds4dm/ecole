.. _create-new-functions:

Create New Functions
====================

:py:class:`~ecole.typing.ObservationFunction` and :py:class:`~ecole.typing.RewardFunction` functions
can be adapted and created from Python.

At the core of the environment, a SCIP :py:class:`~ecole.scip.Model` (equivalent abstraction to a
``pyscipopt.Model`` or a ``SCIP*`` in ``C``), describes the state of the environment.
The idea of observation and reward functions is to have a function that takes as input a
:py:class:`~ecole.scip.Model`, and returns the desired value (an observation, or a reward).
The environment itself does nothing more than calling the functions and forward their output to the
user.

Pratically speaking, it is more convenient to implement such functions as a class than a function,
as it makes it easier to keep information between states.

Extending a Function
--------------------
To reuse a function, Python inheritance can be used. For example, the method in an observation function called
to extract the features from the model is called :py:meth:`~ecole.typing.ObservationFunction.extract`.
In the following example, we will extend the :py:class:`~ecole.observation.NodeBipartite` observation function by
overloading its :py:meth:`~ecole.typing.ObservationFunction.extract` function to scale the features by their
maximum absolute value.

.. testcode::

   import numpy as np
   from ecole.observation import NodeBipartite


   class ScaledNodeBipartite(NodeBipartite):

       def extract(self, model, done):
           # Call parent method to get the original observation
           obs = super().extract(model, done)
           # Apply scaling
           column_max_abs = np.abs(obs.column_features).max(0)
           obs.column_features[:] /= column_max_abs
           row_max_abs = np.abs(obs.row_features).max(0)
           obs.row_features[:] /= row_max_abs
           # Return the updated observation
           return obs

By using inheritance, we used :py:class:`~ecole.observation.NodeBipartite`'s own :py:meth:`~ecole.typing.ObservationFunction.extract`
to do the heavy lifting, only appending the additional scaling code.
The resulting ``ScaledNodeBipartite`` class is a perfectly valid observation function that can be given to an
environment.

As an additional example, instead of scaling by the maximum absolute value one might want to use a scaling factor smoothed by
`exponential moving averaging <https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average>`_, with some coefficient α.
This will illustrate how the class paradigm is useful to saving information between states.

.. testcode::

   class MovingScaledNodeBipartite(NodeBipartite):

       def __init__(self, alpha, *args, **kwargs):
           # Construct parent class with other parameters
           super().__init__(*args, **kwargs)
           self.alpha = alpha

       def before_reset(self, model):
           super().before_reset(model)
           # Reset the exponential moving average (ema) on new episodes
           self.column_ema = None
           self.row_ema = None

       def extract(self, model, done):
           obs = super().extract(model, done)

           # Compute the max absolute vector for the current observation
           column_max_abs = np.abs(obs.column_features).max(0)
           row_max_abs = np.abs(obs.row_features).max(0)

           if self.column_ema is None:
               # New exponential moving average on a new episode
               self.column_ema = column_max_abs
               self.row_ema = row_max_abs
           else:
               # Update the exponential moving average
               self.column_ema = self.alpha * column_max_abs + (1 - alpha) * self.column_ema
               self.row_ema = self.alpha * row_max_abs + (1 - alpha) * self.row_ema

           # Scale features and return the new observation
           obs.column_features[:] /= self.column_ema
           obs.row_features[:] /= self.row_ema
           return obs

Here, you can notice how we used the constructor to customize the coefficient of the
exponential moving average.
Note also that we inherited the :py:meth:`~ecole.typing.ObservationFunction.before_reset` method which does not
return anything: this method is called at the begining of the episode by
:py:meth:`~ecole.environment.Environment.reset` and is used to reintialize the class
internal attribute on new episodes.
Finally, the :py:meth:`~ecole.typing.ObservationFunction.extract` is also called during during
:py:meth:`~ecole.environment.Environment.reset`, hence the ``if`` else ``else`` condition.
Both these methods call the parent method to let it do its own initialization/resetting.

.. warning::

   The scaling shown in this example is naive implementation meant to showcase the use of
   observation function.
   For proper scaling functions consider `Scikit-Learn Scalers
   <https://scikit-learn.org/stable/modules/classes.html#module-sklearn.preprocessing>`_


Writing a Function from Scratch
-------------------------------
The :py:class:`~ecole.typing.ObservationFunction` and :py:class:`~ecole.typing.RewardFunction` classes don't do
anything more than what is explained in the previous section.
This means that to create new function in Python, one can simply create a class with the previous
methods.

For instance, we can create a ``StochasticReward`` function that will wrap any given
:py:class:`~ecole.typing.RewardFunction`, and with some probability return either the given reward or
0.

.. testcode::

   import random


   class StochasticReward:

       def __init__(self, reward_function, probability = 0.05):
           self.reward_function = reward_function
           self.probability = probability

       def before_reset(self, model):
           self.reward_function.before_reset(model)

       def extract(self, model, done):
           # Unconditionally getting reward as reward_funcition.extract may have side effects
           reward = self.reward_function.extract(model, done)
           if random.random() < probability:
               return 0.
           else:
               return reward

The resulting class is a perfectly valid reward function which can be used in any environment, for example as follows.

.. doctest::

   >> stochastic_lpiterations = StochaticReward(-ecole.reward.LpIteration, probability=0.1)
   >> env = ecole.environment.Branching(reward_function=stochastic_lpiterations)


Using PySCIPOpt
---------------
The extraction functions described on this page, by definition, aim to extract information from the solver about the state
of the process. An excellent reason to create or extend a reward function is to access information not provided by the
default functions in Ecole. To do so in Python, one might want to use `PyScipOpt <https://github.com/SCIP-Interfaces/PySCIPOpt>`_,
the official Python interface to SCIP.

In ``PySCIPOpt`, the state of the SCIP solver is stored in an ``pyscipopt.Model`` object. This is closely related to,
but not quite the same, as Ecole's :py:class:`~ecole.scip.Model` class. For a number of reasons (such as C++ compatibility),
the two classes don't coincide. However, for ease of use, it is possible to convert back and forth without any copy.

Using :py:meth:`ecole.scip.Model.as_pyscipopt`, one can get a ``pyscipopt.Model`` that shares its
internal data with :py:class:`ecole.scip.Model`. Conversely, given a ``pyscipopt.Model``, it is possible to to create a :py:class:`ecole.scip.Model`
using the static method :py:meth:`ecole.scip.Model.from_pyscipopt`.
