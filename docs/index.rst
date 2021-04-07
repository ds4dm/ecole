Introduction
============

Ecole is a library of *Extensible Combinatorial Optimization Learning Environments*
designed to ease the development of machine learning approaches for
combinatorial optimization. More precisely, the goal of Ecole is to allow for a fast
and safe prototyping of any ML for CO approach that can be formulated as a control
problem (*i.e.*, a Markov Decision Problem), as well as providing reproducible benchmarking protocols
for comparison to existing approaches.

.. testcode::

   import ecole

   env = ecole.environment.Branching(
       reward_function=-1.5 * ecole.reward.LpIterations() ** 2,
       observation_function=ecole.observation.NodeBipartite(),
   )
   instances = ecole.instance.SetCoverGenerator(n_rows=100, n_cols=200)

   for _ in range(10):
       observation, action_set, reward_offset, done, info = env.reset(next(instances))
       while not done:
           observation, action_set, reward, done, info = env.step(action_set[0])


Combinatorial optimization solvers typically rely on a plethora of handcrafted expert heuristics,
which can fail to exploit subtle statistical similarities between problem intances.
`Machine Learning <https://en.wikipedia.org/wiki/Machine_learning>`_ algorithms offer
a promising candidate for replacing those heuristics, by learning data-driven policies that automatically
account for such statistical relationships, and thereby creating a new kind of highly adaptive solvers.

For instance, many combinatorial optimization problems can be modeled using `Mixed Integer
Linear Programming <https://en.wikipedia.org/wiki/Integer_programming>`_ and solved using
the `branch-and-bound <https://en.wikipedia.org/wiki/Branch_and_bound>`_ algorithm.
Despite its simplicity, the algorithm requires many non-trivial decisions, such as iteratively
picking the next variable to branch on. Ecole aims at exposing these algorithmic control problems with a
standard reinforcement learning API (agent / environment loop), in order to ease the exploration
of new machine learning models and algorithms for learning data-driven policies.

Ecole's interface is inspired from `OpenAi Gym <https://gym.openai.com/>`_ and will look
familiar to reinforcement learning praticionners.
The state-of-the-art Mixed Integer Linear Programming solver that acts as a controllable
algorithm inside Ecole is `SCIP <https://scip.zib.de/>`_.

The reader is referred to [Bengio2020]_ for motivation on why machine learning is a promising
candidate to use for combinatorial optimization, as well as the methodology to do so.

.. [Bengio2020]
   Bengio, Yoshua, Andrea Lodi, and Antoine Prouvost.
   "`Machine learning for combinatorial optimization: a methodological tour d'horizon.
   <https://arxiv.org/pdf/1811.06128.pdf>`_"
   *European Journal of Operational Research*. 2020.


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
   howto/instances.rst

.. toctree::
   :caption: Practical Tutorials
   :hidden:

   Configuring the Solver with Bandits <https://github.com/ds4dm/ecole/tree/master/examples/configuring-bandits.ipynb>
   Branching with Imitation Learning <https://github.com/ds4dm/ecole/tree/master/examples/branching-imitation.ipynb>

.. toctree::
   :caption: Reference
   :hidden:

   reference/environments.rst
   reference/observations.rst
   reference/rewards.rst
   reference/information.rst
   reference/scip-interface.rst
   reference/instances.rst
   reference/utilities.rst

.. toctree::
   :caption: Discussion
   :hidden:

   discussion/gym-differences.rst
   discussion/seeding.rst
   discussion/theory.rst

.. toctree::
   :caption: Developer Zone
   :hidden:

   contributing.rst
   developers/example-observation.rst
