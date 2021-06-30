.. _use-observation-functions:

Use Observation Functions
=========================

Using any environment, the observation [#observation]_ received by the user to take the
next action can be customized changing the :py:class:`~ecole.typing.ObservationFunction` used by the solver.
The environment is not extracting data directly but delegates that responsibility to an
:py:class:`~ecole.typing.ObservationFunction` object.
The object has complete access to the solver and extract the data it needs.

Specifying an observation function is as easy as specifying a parameter when
creating an environment.
For instance with the :py:class:`~ecole.environment.Branching` environment:

.. doctest::

   >>> env = ecole.environment.Branching(observation_function=ecole.observation.Nothing())
   >>> env.observation_function  # doctest: +SKIP
   ecole.observation.Nothing()
   >>> obs, _, _, _, _ = env.reset("path/to/problem")
   >>> obs is None
   True

Environments have an observation function set as default parameter for convenience.

.. doctest::

   >>> env = ecole.environment.Branching()
   >>> env.observation_function  # doctest: +SKIP
   ecole.observation.NodeBipartite()
   >>> obs, _, _, _, _ = env.reset("path/to/problem")
   >>> obs  # doctest: +SKIP
   ecole.observation.NodeBipartiteObs(...)

.. TODO Use an observation function that is more intutive than Nothing
.. TODO Adapt the output to the actual __repr__ and remove #doctest: +SKIP


See :ref:`the reference<observation-reference>` for the list of available observation functions,
as well as :ref:`the documention<create-new-functions>` for explanation on how to create one.


No Observation Function
-----------------------
To not use any observation function, for instance for learning with a bandit algorithm,
you can explicitly pass ``None`` to the environment constructor.

.. doctest::

   >>> env = ecole.environment.Branching(observation_function=None)
   >>> env.observation_function  # doctest: +SKIP
   ecole.observation.nothing()
   >>> obs, _, _, _, _= env.reset("path/to/problem")
   >>> obs is None
   True

.. TODO Adapt the output to the actual __repr__ and remove #doctest: +SKIP

Multiple Observation Functions
------------------------------
To use multiple observation functions, wrap them in a ``list`` or ``dict``.

.. doctest::

   >>> obs_func = {
   ...    "some_name": ecole.observation.NodeBipartite(),
   ...    "other_name": ecole.observation.Nothing(),
   ... }
   >>> env = ecole.environment.Branching(observation_function=obs_func)
   >>> obs, _, _, _, _ = env.reset("path/to/problem")
   >>> obs  # doctest: +SKIP
   {'some_name': ecole.observation.NodeBipartiteObs(), 'other_name': None}

.. TODO Adapt the output to the actual __repr__ and remove #doctest: +SKIP

Similarily with a tuple

.. doctest::

   >>> obs_func = (
   ...    ecole.observation.NodeBipartite(), ecole.observation.Nothing()
   ... )
   >>> env = ecole.environment.Branching(observation_function=obs_func)
   >>> obs, _, _, _, _ = env.reset("path/to/problem")
   >>> obs  # doctest: +SKIP
   [ecole.observation.NodeBipartiteObs(), None]

.. TODO Use an observation function that is more intutive than Nothing
.. TODO Adapt the output to the actual __repr__ and remove #doctest: +SKIP

.. [#observation] We use the term *observation* rather than state since the state
  is really the whole state of the solver, which is unaccessible. Thus, mathematically,
  we really have a Partially Observable Markov Decision Process.
