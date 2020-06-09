Introduction
============

Ecole is a library of *Extensible Combinatorial Optimization Learning Environments*
designed to facilitate the definition of machine learning environments problems inside
combinatorial optimization solvers.

.. code-block:: python

   import ecole

   env = ecole.environment.Branching(
       reward_function=-ecole.reward.LpIterations(),
       observation_function=ecole.observation.NodeBipartite()
   )

   for _ in range(10):
        observation, action_set, done = env.reset("path/to/problem")
        while not done:
            obs, action_set, reward, done, info = env.step(action_set[0])


Combinatorial optimization solvers rely on a variety of hand crafted heuristic that fail
to account for similarities in between problems.
`Machine Learning <https://en.wikipedia.org/wiki/Machine_learning>`_ algorithms are
a promising candidate to create a new kind of highly adaptive solvers that can adapt to
the problem data.

For instance, many combinatorial optimization problems can be modeled using `Mixed Integer
Linear Programming <https://en.wikipedia.org/wiki/Integer_programming>`_ and solved using
the `branch-and-bound <https://en.wikipedia.org/wiki/Branch_and_bound>`_ algorithm.
This algorithm still requires many more decisions, such as picking the variable to branch
on.
Ecole let the user easily explore new policies to make these decisions using machine
learning and information extracted from the solver.

Ecole's interface is inspired from `OpenAi Gym <https://gym.openai.com/>`_ and will look
familiar to reinforcement learning praticionners.
The state-of-the-art Mixed Integer Linear Programming solver that acts as a controllable
algorithm inside Ecole is `SCIP <https://scip.zib.de/>`_.

The reader is referred to [Bengio2018]_ for motiation on why machine learning is a promising
candidate to use for combinatorial optimization, as well as the methodology to do so.

.. [Bengio2018]
   Bengio, Yoshua, Andrea Lodi, and Antoine Prouvost.
   "Machine Learning for Combinatorial Optimization: a Methodological Tour d'Horizon."
   arXiv preprint arXiv:1811.06128 (2018).


.. toctree::
   :caption: Getting started
   :hidden:

   self
   installation
   using-environments

.. toctree::
   :caption: How to
   :hidden:

   howto/observation-functions.rst
   howto/reward-functions.rst
   howto/create-functions.rst
   howto/create-environments.rst

.. toctree::
   :caption: Tutorials
   :hidden:

   Configuring the Solver with Bandits <https://github.com/ds4dm/ecole/tree/master/examples/configuring-bandits.ipynb>
   Branching with Imitation Learning <https://github.com/ds4dm/ecole/tree/master/examples/branching-imitation.ipynb>

.. toctree::
   :caption: Reference
   :hidden:

   reference/environments.rst
   reference/observations.rst
   reference/rewards.rst

.. toctree::
   :caption: Discussion
   :hidden:

   discussion/gym-differences.rst
   discussion/seeding.rst
   discussion/theory.rst
