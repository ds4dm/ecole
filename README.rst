.. image:: docs/_static/images/ecole-logo.svg
   :target: https://www.ecole.ai
   :alt: Ecole logo
   :width: 30 %
   :align: right

Ecole
=====

.. image:: https://circleci.com/gh/ds4dm/ecole.svg?style=svg
   :target: https://circleci.com/gh/ds4dm/ecole
   :alt: CircleCI


Ecole (pronounced [ekɔl]) stands for *Extensible Combinatorial Optimization Learning
Environments* and aims to expose a number of control problems arising in combinatorial
optimization solvers as Markov
Decision Processes (*i.e.*, Reinforcement Learning environments).
Rather than trying to predict solutions to combinatorial optimization problems directly, the
philosophy behind Ecole is to work
in cooperation with a state-of-the-art Mixed Integer Linear Programming solver
that acts as a controllable algorithm.

The underlying solver used is `SCIP <https://scip.zib.de/>`_, and the user facing API is
meant to mimic the `OpenAi Gym <https://gym.openai.com/>`_ API (as much as possible).

.. code-block:: python

   import ecole

   env = ecole.environment.Branching(
       reward_function=-1.5 * ecole.reward.LpIterations() ** 2,
       observation_function=ecole.observation.NodeBipartite(),
   )
   instances = ecole.instance.SetCoverGenerator()

   for _ in range(10):
       obs, action_set, reward_offset, done, info = env.reset(next(instances))
       while not done:
          obs, action_set, reward, done, info = env.step(action_set[0])


- Consult the `user Documentation <https://doc.ecole.ai>`_ for tutorials, examples, and library reference.
- Head to `Github Discussions <https://github.com/ds4dm/ecole/discussions>`_ for interaction with the community: give
  and recieve help, discuss intresting envirnoment, rewards function, and instances generators.

Installation
------------
Conda
^^^^^
.. code-block:: bash

   conda install -c scipopt -c conda-forge ecole

`PyScipOpt <https://github.com/SCIP-Interfaces/PySCIPOpt>`_ is not required but is the main SCIP
interface to develop new Ecole components from Python

.. code-block:: bash

   conda install -c scipopt -c conda-forge ecole pyscipopt

Currenlty, conda packages are only available for Linux and MacOS.

Checkout the `installation instructions <#Installation>`_ (on this page).

Pip
^^^
Currently unavailable

From Source
^^^^^^^^^^^
Source builds currently require following the contributing instructions.


Use It, Cite It
---------------
If you use Ecole in a scientific publication, please cite the
`Ecole publication <https://arxiv.org/abs/2011.06069>`_.

.. code-block:: text

   @article{prouvost2020ecole,
     title={Ecole: A Gym-like Library for Machine Learning in Combinatorial Optimization Solvers},
     author={Prouvost, Antoine and Dumouchelle, Justin and Scavuzzo, Lara and Gasse, Maxime and Ch{\'e}telat, Didier and Lodi, Andrea},
     journal={arXiv preprint arXiv:2011.06069},
     year={2020}
   }
