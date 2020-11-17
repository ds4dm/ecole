.. _reward-reference:

Rewards
=======

Protocol
--------
The protocol expected to define a valid reward function is given below.

.. autoclass:: ecole.typing.RewardFunction

Listing
-------
The list of reward functions relevant to users is given below.

Is Done
^^^^^^^
.. autoclass:: ecole.reward.IsDone
   :no-members:
   :members: reset, extract

LP Iterations
^^^^^^^^^^^^^
.. autoclass:: ecole.reward.LpIterations
   :no-members:
   :members: reset, extract

NNodes
^^^^^^
.. autoclass:: ecole.reward.NNodes
   :no-members:
   :members: reset, extract


Utilities
---------
The following reward functions are used internally by Ecole.

Constant
^^^^^^^^
.. autoclass:: ecole.reward.Constant
   :no-members:
   :members: reset, extract

Arithmetic
^^^^^^^^^^
.. autoclass:: ecole.reward.Arithmetic
   :no-members:
   :members: reset, extract
