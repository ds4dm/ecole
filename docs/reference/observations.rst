.. _observation-reference:

Observations
============

Protocol
--------
The protocol expected to define a valid observation function is given below.

.. autoclass:: ecole.typing.ObservationFunction


Listing
-------
The list of observation functions relevant to users is given below.

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

Utilities
---------
The following observation functions are used internally by Ecole.

Nothing
^^^^^^^
.. autoclass:: ecole.observation.Nothing

Tuple of Observations
^^^^^^^^^^^^^^^^^^^^^
.. autoclass:: ecole.observation.TupleFunction

Dictionnary of Observations
^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. autoclass:: ecole.observation.DictFunction
