Adapting Environments
=====================
Contrarily to `OpenAI Gym <https://gym.openai.com/>`_ where learning tasks are predefined,
Ecole gives the user the tools to easily extend and customize environments.
This is because the objective with Ecole is not only to provide a collection of challenges
for machine learning, but really to solve combinatorial optimization problems more
efficiently.
If different data or changing the tweaking the control task delivers better performances,
it is an improvement!


Observation Functions
---------------------
Using any environment, the observation [#observation]_ recieved by the user to take the
next action can be changing the ``ObservationFunction`` used by the solver.
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


Other Constructor Arguments
---------------------------
