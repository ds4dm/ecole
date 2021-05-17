.. image:: docs/_static/images/ecole-logo.svg
   :target: https://www.ecole.ai
   :alt: Ecole logo
   :width: 30 %
   :align: right

Ecole
=====

.. image:: https://github.com/ds4dm/ecole/actions/workflows/continuous-testing.yml/badge.svg
   :target: https://github.com/ds4dm/ecole/actions/workflows/continuous-testing.yml
   :alt: Test and deploy on Github Actions

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

.. image:: https://img.shields.io/conda/vn/conda-forge/ecole?label=version&logo=conda-forge
   :target: https://anaconda.org/conda-forge/ecole
   :alt: Conda-Forge version
.. image:: https://img.shields.io/conda/pn/conda-forge/ecole?logo=conda-forge
   :target: https://anaconda.org/conda-forge/ecole
   :alt: Conda-Forge platforms

.. code-block:: bash

   conda install -c conda-forge ecole

All dependencies are resolved by conda, no compiler is required.

Pip wheel (binary)
^^^^^^^^^^^^^^^^^^
Currently unavailable.

Pip source
^^^^^^^^^^^
.. image:: https://img.shields.io/pypi/v/ecole?logo=python
   :target: https://pypi.org/project/ecole/
   :alt: PyPI version

Building from source requires:
 - A `C++17 compiler <https://en.cppreference.com/w/cpp/compiler_support>`_,
 - A `SCIP <https://www.scipopt.org/>`__ installation.

.. code-block:: bash

   pip install ecole

Other Options
^^^^^^^^^^^^^
Checkout the `installation instructions <https://doc.ecole.ai/master/installation.html>`_ in the
documentation for more installation options.

Related Projects
----------------

* `OR-Gym <https://github.com/hubbs5/or-gym>`_ is a gym-like library providing gym-like environments to produce feasible solutions
  directly, without the need for an MILP solver;
* `MIPLearn <https://github.com/ANL-CEEESA/MIPLearn>`_ for learning to configure solvers.

Use It, Cite It
---------------

.. image:: https://img.shields.io/badge/arxiv-2011.06069-red
   :target: https://arxiv.org/abs/2011.06069
   :alt: Ecole publication on Arxiv


If you use Ecole in a scientific publication, please cite the Ecole publication

.. code-block:: text

   @inproceedings{
       prouvost2020ecole,
       title={Ecole: A Gym-like Library for Machine Learning in Combinatorial Optimization Solvers},
       author={Antoine Prouvost and Justin Dumouchelle and Lara Scavuzzo and Maxime Gasse and Didier Ch{\'e}telat and Andrea Lodi},
       booktitle={Learning Meets Combinatorial Algorithms at NeurIPS2020},
       year={2020},
       url={https://openreview.net/forum?id=IVc9hqgibyB}
   }
