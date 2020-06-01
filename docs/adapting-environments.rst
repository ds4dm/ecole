Adapting Environments
=====================
Contrarily to `OpenAI Gym <https://gym.openai.com/>`_ where learning tasks are predefined,
Ecole gives the user the tools to easily extend and customize environments.
This is because the objective with Ecole is not only to provide a collection of challenges
for machine learning, but really to solve combinatorial optimization problems more
efficiently.
If different data or changing the tweaking the control task delivers better performances,
it is an improvement!


.. _observation-functions:

Observation Functions
---------------------
Using any environment, the observation [#observation]_ recieved by the user to take the
next action can be customized changing the ``ObservationFunction`` used by the solver.
The environment is not extracting data directly but delegates that responsibility to an
``ObservationFunction`` object.
The object has complete access to the solver and extract the data it needs.

.. TODO Add reference and docstring for observation functions

Using a different observation function is as easy as passing it as a parameter when
creating an environment.
For instance with the :py:class:`~ecole.environment.Branching` environment:

.. code-block:: python

   >>> env = ecole.environment.Branching(observation_function=ecole.observation.Nothing())
   >>> env.observation_function
   ecole.observation.Nothing()
   >>> obs, _, _ = env.reset("path/to/problem")
   >>> obs
   None

Environments have an observation function set as default parameter for convenience.

.. code-block:: python

   >>> env = ecole.environment.Branching()
   >>> env.observation_function
   ecole.observation.NodeBipartite()
   >>> obs, _, _ = env.reset("path/to/problem")
   >>> obs
   NodeBipartiteObs(...)

.. TODO Use an observation function that is more intutive than Nothing
.. TODO Adapt the output to the actual __repr__

See [TODO] for the list of available observation function, as well as [TODO] for
explanation on how to create one.

.. TODO Fill the missing references

No Observation Function
^^^^^^^^^^^^^^^^^^^^^^^
To not use any observation function, for instance for a learning with a bandit algorithm,
explicitly pass ``None`` to the environment constructor.

.. code-block:: python

   >>> env = ecole.environment.branching(observation_function=None)
   >>> env.observation_function
   ecole.observation.nothing()
   >>> obs, _, _ = env.reset("path/to/problem")
   >>> obs is None
   True

Multiple Observation Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To use multiple observation functions, wrap them in a ``tuple`` or ``dict``.

.. code-block:: python

   >>> obs_func = (
   ...    ecole.observation.NodeBipartite(), ecole.observation.Nothing()
   ... )
   >>> env = ecole.environment.branching(observation_function=obs_func)
   >>> obs, _, _ = env.reset("path/to/problem")
   >>> obs
   (ecole.observation.NodeBipartiteObs(), None)

Similarily with a tuple

.. code-block:: python

   >>> obs_func = {
   ...    "some_name": ecole.observation.NodeBipartite(),
   ...    "other_name": ecole.observation.Nothing(),
   ... }
   >>> env = ecole.environment.branching(observation_function=obs_func)
   >>> obs, _, _ = env.reset("path/to/problem")
   >>> obs
   {'some_name': ecole.observation.NodeBipartiteObs(), 'other_name': None}

.. TODO Use an observation function that is more intutive than Nothing
.. TODO Adapt the output to the actual __repr__

.. [#observation] We chose to use *observation*, according to the Partially Observable
   Markov Decision Process, because the state is really the whole state of the solver.


Reward Functions
----------------
Similarily to :ref:`observation functions <observation-functions>` the reward recieved by
the user for learning can be customized by changing the ``RewardFunction`` used by the
solver.
In fact the mechanism of reward functions is very similar to that of observation
functions.
Likewise environment is not computing the reward directly but delegates that
responsibility to a ``RewardFunction`` object.
The object has complete access to the solver and extract the data it needs.

.. TODO Add reference and docstring for reward functions

Using a different reward function is done with another parameter to the environment.
For instance with the :py:class:`~ecole.environment.Configuring` environment:

.. code-block:: python

   >>> env = ecole.environment.Configuring(reward_function=ecole.reward.LpIiterations())
   >>> env.reward_function
   ecole.reward.LpIterations()
   >>> env.reset("path/to/problem")
   >>> _, _, reward, _, _ = env.step({})
   4560.0

Environments also have a default reward function.

.. code-block:: python

   >>> env = ecole.environment.Configuring()
   >>> env.reward_function
   ecole.reward.IsDone()

.. TODO Adapt the output to the actual __repr__

See [TODO] for the list of available reward functions, as well as [TODO] for explanation
on how to create one.

Arithmetic on Reward Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Finding a good reward function that will keep the learning process stable and efficient is
a complex and active area of research.
When dealing with new types of data, as is the case with Ecole, it is even more important
to explore differents rewards.
To create and combine rewards, python arithmetic operations can be used.

For instance, one typically want to minimize the number of
:py:class:`~ecole.reward.LpIterations`.
To achieve this, one would typically use the opposite of the reward.
Such a reward function can be created by negating the reward function.

.. code-block:: python

   >>> env = ecole.environment.Configuring(reward_function=-ecole.reward.LpIiterations())
   >>> env.reset("path/to/problem")
   >>> _, _, reward, _, _ = env.step({})
   -4560.0

Any operation, such as

.. code-block:: python

   -3.5 * LpIiterations() ** 2.1 + 4.4

are valid.

Note that this is a full reward *function* object that can be given to an environment.
it is similar to doing the following

.. code-block:: python

   >>> env = ecole.environment.Configuring(reward_function=ecole.reward.LpIiterations())
   >>> env.reset("path/to/problem")
   >>> _, _, lp_iter_reward, _, _ = env.step({})
   >>> reward = -3.5 * lp_iter_reward ** 2.1 + 4.4

Arithmetic operations on reward functions become exremely powerful when combining mutiple
rewards functions, such as in

.. code-block:: python

   4.0 * LpIterations()**2 - 3 * IsDone()

because in this case it would *not* be possible to pass both
:py:class:`~ecole.reward.LpIterations` and :py:class:`~ecole.reward.IsDone` to the
environment.

All operations that are valid between scalars are valid with reward functions

.. code-block:: python

   - IsDone() ** abs(LpIteration() // 4)

Not all mathematical operations have a dedicated Python operator.
Ecole implements a number of other operations are as methods to reward functions.
For instance, to get the exponential of :py:class:`~ecole.reward.LpIterations`, one can
use

.. code-block:: python

   LpIterations().exp()

This also works with rewards functions created from any expression

.. code-block:: python

   (3 - 2*LpIterations()).exp()

In last resort, reward functions have an ``apply`` method to compose rewards with any
function

.. code-block:: python

   LpIterations().apply(lambda reward: math.factorial(round(reward)))


Other Constructor Arguments
---------------------------
