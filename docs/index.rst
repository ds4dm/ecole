Introduction
============

Ecole (pronounced [ekɔl]) stands for *Extensible Combinatorial Optimization Learning
Environments*.
It is a library to expose and define control tasks arising in combinatorial
optimization solvers as Markov Decision Processes (*i.e.*, `Reinforcement Learning
<https://en.wikipedia.org/wiki/Reinforcement_learning>`_ environments).

For instance, many combinatorial optimization problems can be modeled using `Mixed Integer
Linear Programming <https://en.wikipedia.org/wiki/Integer_programming>`_ and solved using
the `branch-and-bound <https://en.wikipedia.org/wiki/Branch_and_bound>`_ algorithm.
This algorithm still requires many more decisions, such as picking the variable to branch
on.
Ecole let the user easily explore new policies to make these decisions using information
extracted from the solver.
Typically the goal for the user is to use `Machine Learning
<https://en.wikipedia.org/wiki/Machine_learning>`_ to make automated decision based on
the solver internal state.

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
   :caption: Usage
   :hidden:

   adapting-environments
   pyscipopt

.. toctree::
   :caption: Reference
   :hidden:

   reference/environments.rst
   reference/observations.rst
   reference/rewards.rst
