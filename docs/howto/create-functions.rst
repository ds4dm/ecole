Create New Functions
====================

::ref:`Observation <use-observation-functions>` and ::ref:`reward <use-reward-functions>` functions
can be adapted and created from Python.

.. TODO add ref to Model

At the core of the environment, a SCIP ``Model`` (equivalent to a ``pyscipopt.Model`` or a
``SCIP*`` in ``C``), describe the state of the environment.
The idea of observation and reward functions is to have a function that takes as input that
``Model``, and return the desired value (an observation, or a reward).
The environment itself does nothing more than calling the function and forward its output to the
user.

Pratically speaking, it is more convenient to implement such functions as a class that a function,
as it makes it easier to keep information between states.

.. TODO protocol reference

From an Exsiting One
--------------------
To reuse a function, Python inheritance can be use.
In the following, we will adapt :py:class:`~ecole.observation.NodeBipartite` to apply some scaling
to the observation features.

.. TODO reference obtain_observation protocol

The method that will be called to return an observation is called ``obtain_observation``.
Here is how we can create a new observation function that scale the features their maximum absolute
value.

.. code-block:: python

   import numpy as np
   from ecole.observation import NodeBipartite


   class ScaledNodeBipartite(NodeBipartite):

       def obtain_observation(self, model):
           # Call parent method to get the original observation
           obs = super().obtain_observation(model)
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

.. code-block:: python

   class MovingScaledNodeBipartite(NodeBipartite):

       def __init__(self, alpha, *args, **kwargs):
           # Construct parent class with other parameters
           super().__init__(*args, **kwargs)
           self.alpha = alpha

       def reset(self, model):
           super().reset(model)
           # Reset exponential moving average (ema) on new episode
           self.column_ema = None
           self.row_ema = None

       def obtain_observation(self, model):
           obs = super().obtain_observation(model)

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

.. TODO reference reset protocol

Here, you can notice how we used the constructor to be able to customize the coefficient of the
exponential moving average.
We also inherited the ``reset`` method which does not return anything.
This method is called at the begining of the episode by
:py:meth:`~ecole.environment.EnvironmentComposer.reset` and is used to reintialize the class
internal attribute on new episodes.
The ``obtain_observation`` is also called during during
:py:meth:`~ecole.environment.EnvironmentComposer.reset`, hence the ``if`` else ``else`` condition.
Both these methods call the parent method to let it do its own initialization/reseting.

.. warning::

   The scaling shown in this example is naive implementation meant to showcase the use of
   observation function.
   For proper scaling functions consider `Scikit-Learn Scalers
   <https://scikit-learn.org/stable/modules/classes.html#module-sklearn.preprocessing>`_



From Scratch
------------
