.. _create-new-functions:

Create New Functions
====================

:py:class:`~ecole.typing.ObservationFunction` and :py:class:`~ecole.typing.RewardFunction` functions
can be adapted and created from Python.

At the core of the environment, a SCIP :py:class:`~ecole.scip.Model` (equivalent abstraction to a
``pyscipopt.Model`` or a ``SCIP*`` in ``C``), describe the state of the environment.
The idea of observation and reward functions is to have a function that takes as input that
:py:class:`~ecole.scip.Model`, and return the desired value (an observation, or a reward).
The environment itself does nothing more than calling the function and forward its output to the
user.

Pratically speaking, it is more convenient to implement such functions as a class that a function,
as it makes it easier to keep information between states.

From an Exsiting One
--------------------
To reuse a function, Python inheritance can be use.
In the following, we will adapt :py:class:`~ecole.observation.NodeBipartite` to apply some scaling
to the observation features.

The method that will be called to return an observation is called
:py:meth:`~ecole.typing.ObservationFunction.extract`.
Here is how we can create a new observation function that scale the features by their maximum
absolute value.

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

Here we use the :py:class:`~ecole.observation.NodeBipartite` function to do the heavy lifting by
calling the method of the parent class.
Then we scaled some features of that observation and returned the result.
``ScaledNodeBipartite`` is a perfectly valid observation function that can be given to an
environment.

To make it smoother, we could apply an
`exponential moving average <https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average>`_
with coefficient α to the scaling vector.
We will apply the moving average on states from the same episode, and reset it at every new
episode.
This example shows how the scaling vector can be stored between states.

.. testcode::

   class MovingScaledNodeBipartite(NodeBipartite):

       def __init__(self, alpha, *args, **kwargs):
           # Construct parent class with other parameters
           super().__init__(*args, **kwargs)
           self.alpha = alpha

       def before_reset(self, model):
           super().before_reset(model)
           # Reset exponential moving average (ema) on new episode
           self.column_ema = None
           self.row_ema = None

       def extract(self, model, done):
           obs = super().extract(model, done)

           # Compute max absolute vector for current observation
           column_max_abs = np.abs(obs.column_features).max(0)
           row_max_abs = np.abs(obs.row_features).max(0)

           if self.column_ema is None:
               # New exponential moving average on new episode
               self.column_ema = column_max_abs
               self.row_ema = row_max_abs
           else:
               # Update exponential moving average
               self.column_ema = self.alpha * column_max_abs + (1 - alpha) * self.column_ema
               self.row_ema = self.alpha * row_max_abs + (1 - alpha) * self.row_ema

           # Scale features and return new observation
           obs.column_features[:] /= self.column_ema
           obs.row_features[:] /= self.row_ema
           return obs

Here, you can notice how we used the constructor to be able to customize the coefficient of the
exponential moving average.
We also inherited the :py:meth:`~ecole.typing.ObservationFunction.before_reset` method which does not
return anything.
This method is called at the begining of the episode by
:py:meth:`~ecole.environment.Environment.reset` and is used to reintialize the class
internal attribute on new episodes.
The :py:meth:`~ecole.typing.ObservationFunction.extract` is also called during during
:py:meth:`~ecole.environment.Environment.reset`, hence the ``if`` else ``else`` condition.
Both these methods call the parent method to let it do its own initialization/reseting.

.. warning::

   The scaling shown in this example is naive implementation meant to showcase the use of
   observation function.
   For proper scaling functions consider `Scikit-Learn Scalers
   <https://scikit-learn.org/stable/modules/classes.html#module-sklearn.preprocessing>`_


From Scratch
------------
:py:class:`~ecole.typing.ObservationFunction` and :py:class:`~ecole.typing.RewardFunction` do not
anything more than what is explained in the previous section.
This means that to create new function form Python, one can simply create a class with the previous
methods.

For instance, we can create a ``StochasticReward`` function that will wrap any given
:py:class:`~ecole.typing.RewardFunction` and with some probability return either the given reward or
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

It can be used as such, for instance with :py:class:`~ecole.reward.LpIterations` in a
:py:class:`~ecole.environment.Branching` environment.

.. doctest::

   >> stochastic_lpiterations = StochaticReward(-ecole.reward.LpIteration, probability=0.1)
   >> env = ecole.environment.Branching(reward_function=stochastic_lpiterations)


Using PyScipOpt
---------------
When creating a new function, it is common to need to extract information from the solver.
`PyScipOpt <https://github.com/SCIP-Interfaces/PySCIPOpt>`_ is the official Python interface to
SCIP.
The ``pyscipopt.Model`` holds a stateful SCIP problem instance and solver.
For a number of reasons (such as avaibility in C++) Ecole defines its own
:py:class:`~ecole.scip.Model` class that represent a very similar concept.
It does not aim to be a replacement to PyScipOpt, rather it is possible to convert back and forth
without any copy.

Using :py:meth:`ecole.scip.Model.as_pyscipopt`, one can get a ``pyscipopt.Model`` that shares its
internal data with :py:class:`ecole.scip.Model`.

Conversely, given a ``pyscipopt.Model``, it is possible to to create a :py:class:`ecole.scip.Model`
using the static method :py:meth:`ecole.scip.Model.from_pyscipopt`.
