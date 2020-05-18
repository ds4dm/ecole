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

For instance, using the :py:class:`~ecole.environment.Branching` environment for branch-and-bound variable selection,
always selecting the first fractional variable would look like:

.. TODO verify proper link of branching

.. code-block:: python

    import ecole

    environment = ecole.environment.Branching()

    for _ in range(10):
        observation, action_set, done = env.reset("path/to/problem")
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

   The control loop of the markov Decision Process

The outter ``for`` loop only repeats the inner one mutiple times.
Few learning algorithms are able to learn in a single episode, so numerous ones are
usually required.
Usually, although not demonstrated here, one should not use a unique combinatorial problem
instance for all episodes.
This is because an there is no practical interest in solving again an instance already
solved.
One wants to find a policy able to genralize to new, unseen, instances.

.. TODO add ref to theoretical setction


.. _reseting-environments:

Reseting environments
---------------------

The episode in the inner ``while`` starts with a call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset` to bring the environment to a new
initial state.
The problem instance is given to parametrize the episode: it is that combinatorial
optimization problem that will be solved by the `SCIP <https://scip.zib.de/>`_ solver
during the next episode.

The ``observation`` is for the user to decide what the next action will be (typically
using a machine learning algorithm).
The boolean flag ``done`` indicates wether the state is terminal.
This can hapen in :py:class:`~ecole.environment.Branching` where the problem instance
can be resolved though presolving only (never reaching branch-and-bound).
Moreover an ``action_set`` is sometimes given to further reduce the set of candidate
actions to deal with highly dynamic actions sets.
It is valid for the next transition only.
For instance in :py:class:`~ecole.environment.Branching` the set of variable the algorithm
can branch on changes at very node (*i.e.* state).
Therefore the user needs to be constantly given the set of fractional variables.

The exact documentation for the method is given below.

.. automethod:: ecole.environment.EnvironmentComposer.reset


Transitioning
-------------

The inner ``while`` loop transitions the environment from one state to the next by giving
an action to :py:meth:`~ecole.environment.EnvironmentComposer.step`.
The nature of ``observation``, ``action_set``, and ``done`` is the same as in the previous
setction :ref:`reseting-environments`.
Furthermore a ``reward`` and ``info`` variables are given as additional information about
the current transition.

The exact documentation for the method is given below.

.. automethod:: ecole.environment.EnvironmentComposer.step


Seeding environments
--------------------

The exact documention for the method is given below.

.. automethod:: ecole.environment.EnvironmentComposer.seed

.. TODO document this and explain the seeding behaviour
