Using Environments
==================

Environments are a stateful classes representing a control task (or Markov Decision
Process).
After instantiation of an environment, a call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset` will bring the process to its
initial state, then successive calls to
:py:meth:`~ecole.environment.EnvironmentComposer.step` will take an action from the
user and transition to the next state.
When the episode is finished, *i.e* when the combinatorial optimization algorithm
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
The inner ``while`` loop transitions until a terminal state, signaled
with the variable ``done`` in the example, is reached.
This loop matches the following figure usually found in reinforcement learning
presentations.
In Ecole, this loop typically terminates at the same time as the combinatorial
optimization algorithm used in the environment.
A full execution of this loop is known as an *episode*.

.. figure:: images/mdp.png
   :alt: Markov Decision Process interaction loop
   :align: center
   :width: 60%

   The control loop of the Markov Decision Process

The outter ``for`` loop only repeats the inner one mutiple times.
Few learning algorithms are able to learn in a single episode, so numerous ones are
usually required.
Usually, although not demonstrated here, one should not use a unique combinatorial problem
instance for all episodes.
This is because an there is no practical interest in solving again an instance already
solved.
One wants to find a policy able to genralize to new, unseen, instances.

.. TODO add ref to theoretical section


.. _environment-parameters:

Environment parameters
----------------------
Environment can be created with no constructor arguments, as in the previous examples.
This will use all the defaults chosen for the environments.

Each environment can have its own set of parameters to further customize the task being
solved.
For instance, the :py:class:`~ecole.environment.Branching` takes a ``pseudo_candidates``
boolean parameter to decide whether branching candidates are chosen among all non fixed
inegral variables rather than being limited to fractional ones.

The constructor can optionally take a dictionnary of default
`SCIP parameters <https://scip.zib.de/doc/html/PARAMETERS.php>`_ that will be used to
initialize the solver at every episode.
For instance, to customize the clique inequalities generated, one could set:

.. code-block:: python

   env = ecole.environment.Branching(
       scip_params={"separating/clique/freq": 0.5, "separating/clique/maxsepacuts": 5}
   )


.. warning::

   Depending on the nature of the environment, some user given parameters can be overriden
   or ignored (*e.g.* branching parameters in the :py:class:`~ecole.environment.Branching`
   environment).
   It is the responsability of the user to understand the envrionment they are using.

.. note::

   For out-out-the-box strategies on presolving, heurisitcs, and cutting planes, consider
   using the dedicated
   `SCIP methods <https://scip.zib.de/doc/html/group__ParameterMethods.php>`_
   (``SCIPsetHeuristics`` *etc.*).

:ref:`Observation functions <use-observation-functions>` and
:ref:`reward functions <use-observation-functions>` are more advanced environment
parameters dicsussed later on.


.. _reseting-environments:

Reseting environments
---------------------
The episode in the inner ``while`` starts with a call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset` to bring the environment to a new
initial state.
The problem instance is given to parametrize the episode: it is that combinatorial
optimization problem that will be solved by the `SCIP <https://scip.zib.de/>`_ solver
during the next episode.

* The ``observation`` is for the user to decide what the next action will be (typically
  using a machine learning algorithm).
* An ``action_set`` is sometimes given to further reduce the set of candidate
  actions to deal with highly dynamic actions sets.
  It is valid for the next transition only.
  For instance in :py:class:`~ecole.environment.Branching` the set of variable the algorithm
  can branch on changes at very node (*i.e.* state).
  Therefore the user needs to be constantly given the set of fractional variables.
* A ``reward_offset`` is given even though no action has been taken.
  It has not purpose for learning algorithms, rather it is meant for evaluating the complete solving
  procedure.
  When a reward is designed so that its cumulative sum match a metric, such as solving time or number
  of LP iterations, then it is useful to be able to include the computation done during
  :py:meth:`~ecole.environment.EnvironmentComposer.reset`, which are returned in ``reward_offset``.
* The boolean flag ``done`` indicates wether the state is terminal.
  This can hapen in :py:class:`~ecole.environment.Branching` where the problem instance
  can be resolved though presolving only (never reaching branch-and-bound).

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.reset`.


Transitioning
-------------
The inner ``while`` loop transitions the environment from one state to the next by giving
an action to :py:meth:`~ecole.environment.EnvironmentComposer.step`.
The nature of ``observation``, ``action_set``, and ``done`` is the same as in the previous
section :ref:`reseting-environments`.
Furthermore a ``reward`` and ``info`` variables are given as additional information about
the current transition.

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.step`.


Seeding environments
--------------------
Environments can be seeded by using the
:py:meth:`~ecole.environment.EnvironmentComposer.seed` method.
The seed is used by the environment (and in particular the solver) for *all* the
subsequent trajectories.
The solver is given new seeds at the begining of every new trajectory (call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset`) in a way that preserve
determinism, but avoids using the same seeds repeatedly.

See the reference section for the exact documentation of
:py:meth:`~ecole.environment.EnvironmentComposer.seed`.

.. TODO document this and explain the seeding behaviour
