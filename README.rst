.. image:: docs/_static/images/ecole-logo.svg
   :target: https://www.ecole.ai
   :alt: Ecole logo
   :width: 30 %
   :align: right

Ecole
=====

.. image:: https://img.shields.io/circleci/build/github/ds4dm/ecole/master?logo=circleci
   :target: https://circleci.com/gh/ds4dm/ecole
   :alt: CircleCI
.. image:: https://img.shields.io/conda/v/scipopt/ecole
   :target: https://anaconda.org/scipopt/ecole
   :alt: Conda Version
.. image:: https://img.shields.io/conda/pn/scipopt/ecole?label=conda%7Cscipopt
   :target: https://anaconda.org/scipopt/ecole
   :alt: Conda Platforms
   

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


Documentation
-------------
Consult the `user Documentation <https://doc.ecole.ai>`_ for tutorials, examples, and library reference.

Discussions and help
--------------------
Head to `Github Discussions <https://github.com/ds4dm/ecole/discussions>`_ for interaction with the community: give
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

Pip
^^^
Currently unavailable

From Source
-----------
Source builds currently require ``conda`` to fetch the dependencies.

.. code-block:: bash

   conda env create -n ecole -f dev/conda.yaml
   conda activate ecole
   cmake -B build/
   cmake --build build/ --parallel
   python -m pip install build/python


.. warning::

   This mode of installation is not mature.
   In particular, the scip library may not be found when installed outside of the ``ecole`` environemnt.


Use It, Cite It
---------------
If you use Ecole in a scientific publication, please cite the
`Ecole publication <https://arxiv.org/abs/2011.06069>`_.

.. code-block:: text

   @inproceedings{
       prouvost2020ecole,
       title={Ecole: A Gym-like Library for Machine Learning in Combinatorial Optimization Solvers},
       author={Antoine Prouvost and Justin Dumouchelle and Lara Scavuzzo and Maxime Gasse and Didier Ch{\'e}telat and Andrea Lodi},
       booktitle={Learning Meets Combinatorial Algorithms at NeurIPS2020},
       year={2020},
       url={https://openreview.net/forum?id=IVc9hqgibyB}
   }
