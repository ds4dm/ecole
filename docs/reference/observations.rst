.. _observation-reference:

Observations
============

Interface
---------
The interface expected to define a valid observation function is given below.
It is not necessary to inherit from this class, as observation functions are defined by
`structural subtyping <https://mypy.readthedocs.io/en/stable/protocols.html>`_.
It is exists to support Python type hints.

.. autoclass:: ecole.typing.ObservationFunction


Listing
-------
The list of observation functions relevant to users is given below.

Nothing
^^^^^^^
.. autoclass:: ecole.observation.Nothing

Node Bipartite
^^^^^^^^^^^^^^
.. autoclass:: ecole.observation.NodeBipartite
.. autoclass:: ecole.observation.NodeBipartiteObs

Strong Branching Scores
^^^^^^^^^^^^^^^^^^^^^^^
.. autoclass:: ecole.observation.StrongBranchingScores

Pseudocosts
^^^^^^^^^^^
.. autoclass:: ecole.observation.Pseudocosts
