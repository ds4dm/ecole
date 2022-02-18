SCIP Interface
==============

Model
-----
.. autoclass:: ecole.scip.Model

Callbacks
---------
Branchrule
^^^^^^^^^^
.. autoclass:: ecole.scip.callback.BranchruleConstructor
.. autoclass:: ecole.scip.callback.BranchruleCall

Heuristic
^^^^^^^^^
.. autoclass:: ecole.scip.callback.HeuristicConstructor
.. autoclass:: ecole.scip.callback.HeuristicCall

Utilities
^^^^^^^^^
.. autoattribute:: ecole.scip.callback.priority_max
.. autoattribute:: ecole.scip.callback.max_depth_none
.. autoattribute:: ecole.scip.callback.max_bound_distance_none
.. autoattribute:: ecole.scip.callback.frequency_always
.. autoattribute:: ecole.scip.callback.frequency_offset_none

.. autoclass:: ecole.scip.callback.Result
.. autoclass:: ecole.scip.callback.Type

SCIP Data Types
---------------
.. autoclass:: ecole.scip.Stage
.. autoclass:: ecole.scip.HeurTiming
