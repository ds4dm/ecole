Using Environments
==================

Environments are stateful classes representing a control task (or Markov Decision
Process).
After instantiation of an environment, a call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset` will bring the process to its
initial state, then successive calls to
:py:meth:`~ecole.environment.EnvironmentComposer.step` will take an action from the
user and transition to the next state.
When the episode is finished, *i.e.*, when the combinatorial optimization algorithm
terminates, a new one can be started with another call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset`.

For instance, using the :py:class:`~ecole.environment.Branching` environment for
branch-and-bound variable selection, always selecting the first fractional variable would
look like:

.. TODO verify proper link of branching

.. code-block:: python

   import ecole

   env = ecole.environment.Branching()
   env.seed(42)

   for _ in range(10):
       observation, action_set, reward_offset, done = env.reset("path/to/problem")
       while not done:
           obs, action_set, reward, done, info = env.step(action_set[0])


There are few things to note in this example, let us break it down below.


General structure
-----------------
The example is driven by two loops.
The inner ``while`` loop, the so-called *control loop*, transitions from an initial state until a
terminal state is reached, which is signaled with the boolean flag ``done == True``.
In Ecole, the termination of the environment typically coincides with the termination of the
underlying combinatorial optimization algorithm.
A full execution of this loop is known as an *episode*.
The control loop matches the following representation usually found in the reinforcement learning
litterature:

.. figure:: images/mdp.png
   :alt: Markov Decision Process interaction loop
   :align: center
   :width: 60%

   The control loop of a Markov Decision Process

.. note::

   More exactly, the control loop in Ecole is that of a `partially-observable Markov decision process
   <https://en.wikipedia.org/wiki/Partially_observable_Markov_decision_process>`_ (PO-MDP), since
   only a subset of the MDP state is extracted from the environment in the form of an *observation*. We omit
   this detail here for simplicity.

The outter ``for`` loop in the example simply repeats the control loop several times, and is in
charge of generating the initial state of each episode.
In order to obtain a sufficient statistical signal for learning the control policy, numerous episodes are usually required for learning.
Also, although not showcased here, there is usually little practical interest in using the same combinatorial problem
instance for generating each episode. Indeed, it is usually desirable to learn policies that will generalize to new, unseen instances, which is very unlikely if the learning policy is tailored to solve a single specific instance. Ideally, one would like to sample training episodes from a family of similar instances, in order to solve new, similar instances in the future.

.. TODO add ref to theoretical section


.. _environment-parameters:

Environment parameters
----------------------
Each environment can be given a set of parameters at construction, in order to further customize the task being
solved.
For instance, the :py:class:`~ecole.environment.Branching` environment takes a ``pseudo_candidates``
boolean parameter, to decide whether branching candidates should include all non fixed integral variables, or only the fractional ones.
Environments can be instanciated with no constructor arguments, as in the previous example, in which case a set of default parameters will be used.

Every environment can optionally take a dictionnary of
`SCIP parameters <https://scip.zib.de/doc/html/PARAMETERS.php>`_ that will be used to
initialize the solver at every episode.
For instance, to customize the clique inequalities generated, one could set:

.. code-block:: python

   env = ecole.environment.Branching(
       scip_params={"separating/clique/freq": 0.5, "separating/clique/maxsepacuts": 5}
   )


.. warning::

   Depending on the nature of the environment, some user given parameters can be overriden
   or ignored (*e.g.*, branching parameters in the :py:class:`~ecole.environment.Branching`
   environment).
   It is the responsability of the user to understand the environment they are using.

.. note::

   For out-out-the-box strategies on presolving, heuristics, and cutting planes, consider
   using the dedicated
   `SCIP methods <https://scip.zib.de/doc/html/group__ParameterMethods.php>`_
   (``SCIPsetHeuristics`` *etc.*).

:ref:`Observation functions <use-observation-functions>` and
:ref:`reward functions <use-observation-functions>` are more advanced environment
parameters, which we will discuss later on.


.. _reseting-environments:

Reseting environments
---------------------
Each episode in the inner ``while`` starts with a call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset` in order to bring the environment into a new
initial state.
The method is parameterized with a problem instance file: the combinatorial
optimization problem that will be loaded and solved by the `SCIP <https://scip.zib.de/>`_ solver
during the episode.

* The ``observation`` consists in relevant information extracted from the solver,
  for the user to decide what the next action will be (typically
  using a machine learning algorithm).
* The ``action_set``, if present, describes the set of candidate
  actions which are valid for the next transition. This allows us to deal with highly dynamic
  actions sets in a simple way.
  For instance, in the :py:class:`~ecole.environment.Branching` environment the set of candidate variables
  for branching depends on the value of the current LP solution, which changes at every iteration of the algorithm.
* The ``reward_offset`` accounts for any computation happening in
  :py:meth:`~ecole.environment.EnvironmentComposer.reset` when generating the initial state.
  It has no effect on the control problem, and is only given for convenience when the cumulative reward
  of an episode is supposed to match a meaningful combinatorial optimization metric, such as the cumulated running time,
  the number of nodes, the number of LP iterations etc. For example, in :py:class:`~ecole.environment.Branching`
  a substantial computational effort can be spent in :py:meth:`~ecole.environment.EnvironmentComposer.reset`,
  which includes the presolving operation of the solver. That initial reward is not the result of an action though, and therefore
  has no purpose for learning algorithms.
* The boolean flag ``done`` indicates whether the environment immediately reached a terminal state.
  This can happen in :py:class:`~ecole.environment.Branching`, where the problem instance
  can be resolved though presolving only (never actually starting the branch-and-bound algorithm).

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.reset`.


Transitioning
-------------
The inner ``while`` loop transitions the environment from one state to the next by giving
an action to :py:meth:`~ecole.environment.EnvironmentComposer.step`.
The nature of ``observation``, ``action_set``, and ``done`` is the same as in the previous
section :ref:`reseting-environments`.
The ``reward`` and ``info`` variables provide additional information about
the current transition.

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.step`.


Seeding environments
--------------------
Environments can be seeded by using the
:py:meth:`~ecole.environment.EnvironmentComposer.seed` method.
The seed is used by the environment (and in particular the solver) for *all* the
subsequent episode trajectories.
The solver is given a new seed at the begining of every new trajectory (call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset`), in a way that preserves
determinism, without re-using the same seed repeatedly.

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.seed`.

.. TODO document this and explain the seeding behaviour
