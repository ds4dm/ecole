.. _seeding-discussion:

Seeding
=======
Ecole empowers researchers to learn reliable machine learning models, and that means not overfitting
on insignificant behaviours of the solver.
One such aspect is the solver randomness, which is controlled by its random seed.

This means that, by default, Ecole environment will generate different episodes (and in
particular different initial states) after each new call to
:py:meth:`~ecole.environment.EnvironmentComposer.reset`.
To do so, the environment keeps a :py:class:`~ecole.environment.RandomeEngine` (random state)
between episodes, and start a new episode by calling
:py:meth:`~ecole.typing.Dynamics.set_dynamics_random_state` on the underlying
:py:class:`~ecole.typing.Dynamics`.
The latter set random elements of the state including, but not necessary limited to, the
:py:class:`~ecole.scip.Model` random seed, by consuming random numbers from the
:py:class:`~ecole.environment.RandomeEngine`.
That way, the :py:class:`~ecole.typing.EnvironmentComposer` can avoid generating identical
episodes while letting :py:class:`~ecole.typing.Dynamics` decide what random parameters need to
be set.

The :py:meth:`~ecole.environment.EnvironmentComposer.seed` method is really one of the environment,
because it seeds the :py:class:`~ecole.environment.RandomeEngine`., not direclty the episode for
the :py:class:`~ecole.typing.Dynamics`.
Upon creation :py:class:`~ecole.typing.EnvironmentComposer` seed themslef from Python's
`random module <https://docs.python.org/3/library/random.html>`_.

In short we provide the following snippets.

Reproducible program
--------------------
Running this program again will give the same outcome.

.. code-block:: python

   import random
   import ecole

   random.seed(754)

   environment = ecole.environment.Branching()

   for _ in range(10):
       observation, action_set, reward_offset, done = env.reset("path/to/problem")
       while not done:
           obs, action_set, reward, done, info = env.step(action_set[0])


Reproducible environments
-------------------------
Creating this envionment with same seed anywhere else will give the same outcome.

.. code-block:: python

   import ecole

   env = ecole.environment.Branching()
   env.seed(8462)

   for _ in range(10):
       observation, action_set, reward_offset, done = env.reset("path/to/problem")
       while not done:
           obs, action_set, reward, done, info = env.step(action_set[0])


Reproducible episode
--------------------
All episodes run in this snippet are identical.

.. code-block:: python

   import ecole

   env = ecole.environment.Branching()

   for _ in range(10):
       env.seed(81)
       observation, action_set, reward_offset, done = env.reset("path/to/problem")
       while not done:
           obs, action_set, reward, done, info = env.step(action_set[0])
